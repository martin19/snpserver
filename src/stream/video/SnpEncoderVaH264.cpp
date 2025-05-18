#include <util/assert.h>
#include <iostream>
#include <util/VideoUtil.h>
#include <cstring>
#include "util/loguru.h"
#include "SnpEncoderVaH264.h"
#include "va/va.h"
#include "va/va_win32.h"

// https://www.vcodex.com/h264avc-picture-management/

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        fprintf(stderr,"%s:%s (%d) failed with status %d,exit\n", __func__, func, __LINE__, va_status); \
        goto error;                                                        \
    }

void SnpEncoderVaH264::vaInfoCallback([[maybe_unused]] void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}

void SnpEncoderVaH264::vaErrorCallback([[maybe_unused]] void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}

#define FRAME_P 0
#define FRAME_B 1
#define FRAME_I 2
#define FRAME_IDR 7

static unsigned int MaxFrameNum = (2<<4);
static unsigned int MaxPicOrderCntLsb = (2<<16);
static unsigned int Log2MaxFrameNum = 4;
static unsigned int Log2MaxPicOrderCntLsb = 4;

SnpEncoderVaH264::SnpEncoderVaH264(const SnpEncoderVaH264Options &options) : SnpComponent(options, "COMPONENT_ENCODER_INTEL") {
    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGBA));
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_H264));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    //addProperty(new SnpProperty("fps", options.fps));

    width = options.width;
    height = options.height;
    bpp = options.bytesPerPixel;

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderVaH264::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    frameWidthMbAligned = (width + 15) & (~15);
    frameHeightMbAligned = (height + 15) & (~15);
    h264Profile = VAProfileH264ConstrainedBaseline;
    constraintSetFlag = 0;
    frameBitrate = 30000000;
    initialQp = 25;
    minimalQp = 25;

//TODO: this setting works.
//    idrPeriod = 1;
//    iPeriod = 1;
//    ipPeriod = 1;
//TODO: this setting does not work.
    idrPeriod = 60;
    iPeriod = 30;
    ipPeriod = 1;

    configAttribNum = 0;
    currentFrameNum = 0;
    currentFrameType = FRAME_IDR;
    encodingFrameNum = 0;

    numShortTerm = 0;
    numRefFrames = 1;
    LOG_F(INFO, "Initialized.");
}

SnpEncoderVaH264::~SnpEncoderVaH264() {
    //TODO:
}

bool SnpEncoderVaH264::start() {
    SnpComponent::start();
    return initVaH264Encoder();
}

void SnpEncoderVaH264::stop() {
    SnpComponent::stop();
    releaseVaH264Encoder();
}

void SnpEncoderVaH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    if(!isRunning()) return;
    encodeFrameVaH264(data, len);
}

bool SnpEncoderVaH264::initVa() {
    bool result = true;
    VAEntrypoint *entryPoints;
    VAStatus vaStatus;
    int maxNumEntryPoints;
    int numEntryPoints;
    bool supportsEncoding = false;

    int majorVer, minorVer;


//    vaDisplay = vaGetDisplayDRM(getInputPort(0)->deviceFd);
    vaDisplay = vaGetDisplayWin32(nullptr);
    vaSetInfoCallback(vaDisplay, reinterpret_cast<VAMessageCallback>(&SnpEncoderVaH264::vaInfoCallback), nullptr);
    vaSetErrorCallback(vaDisplay, reinterpret_cast<VAMessageCallback>(&SnpEncoderVaH264::vaErrorCallback), nullptr);


    vaStatus = vaInitialize(vaDisplay, &majorVer, &minorVer);
    CHECK_VASTATUS(vaStatus, "vaInitialize")

    maxNumEntryPoints = vaMaxNumEntrypoints(vaDisplay);
    entryPoints = (VAEntrypoint*)calloc(sizeof(VAEntrypoint), maxNumEntryPoints);

    //check for baseline profile and select entryPoint
    vaQueryConfigEntrypoints(vaDisplay, h264Profile, entryPoints, &numEntryPoints);
    for(int i = 0; i < numEntryPoints; i++) {
        if(entryPoints[i] == VAEntrypointEncSlice) {
            selectedEntrypoint = entryPoints[i];
            supportsEncoding = true;
            constraintSetFlag |= (1 << 0 | 1 << 1);
            break;
        }
    }

    ASSERT(supportsEncoding == true);

    LOG_F(INFO, "Found encoder entrypoint. Using profile VAProfileH264ConstrainedBaseline");

    for(int i = 0; i < VAConfigAttribTypeMax; i++) {
        attrib[i].type = (VAConfigAttribType)i;
    }

    vaStatus = vaGetConfigAttributes(vaDisplay, h264Profile, selectedEntrypoint,
                                     &attrib[0],VAConfigAttribTypeMax);
    CHECK_VASTATUS(vaStatus, "vaGetConfigAttributes")

    //discover valid input formats (YUV420 required)
    if((attrib[VAConfigAttribRTFormat].value & VA_RT_FORMAT_YUV420) == 0) {
        fprintf(stderr, "Cannot find desired YUV420 RT format.\n");
        result = false;
        goto error;
    }

    configAttrib[configAttribNum].type = VAConfigAttribRTFormat;
    configAttrib[configAttribNum].value = VA_RT_FORMAT_YUV420;
    configAttribNum++;
    LOG_F(INFO, "Input format YUV420 supported.");

    //discover valid rate control modes (CQP required)
    if(attrib[VAConfigAttribRateControl].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribRateControl].value;
        printf("Support rate control mode (0x%x):", tmp);

        if (tmp & VA_RC_NONE)
            printf("NONE ");
        if (tmp & VA_RC_CBR)
            printf("CBR ");
        if (tmp & VA_RC_VBR)
            printf("VBR ");
        if (tmp & VA_RC_VCM)
            printf("VCM ");
        if (tmp & VA_RC_CQP)
            printf("CQP ");
        if (tmp & VA_RC_VBR_CONSTRAINED)
            printf("VBR_CONSTRAINED ");

        printf("\n");

        if(!(tmp & VA_RC_CQP)) {
            fprintf(stderr, "VA_RC_CQP unsupported!\n");
            result = false;
            goto error;
        }
    }

    configAttrib[configAttribNum].type = VAConfigAttribRateControl;
    configAttrib[configAttribNum].value = VA_RC_CQP;
    configAttribNum++;
    LOG_F(INFO, "Rate control mode CQP supported.");

    if(attrib[VAConfigAttribEncPackedHeaders].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribEncPackedHeaders].value;

        LOG_F(INFO, "Support VAConfigAttribEncPackedHeaders\n");

        configAttrib[configAttribNum].type = VAConfigAttribEncPackedHeaders;
        configAttrib[configAttribNum].value = VA_ENC_PACKED_HEADER_NONE;

        if (tmp & VA_ENC_PACKED_HEADER_SEQUENCE) {
            LOG_F(INFO, "Support packed sequence headers\n");
            configAttrib[configAttribNum].value |= VA_ENC_PACKED_HEADER_SEQUENCE;
        }

        if (tmp & VA_ENC_PACKED_HEADER_PICTURE) {
            LOG_F(INFO,"Support packed picture headers\n");
            configAttrib[configAttribNum].value |= VA_ENC_PACKED_HEADER_PICTURE;
        }

        if (tmp & VA_ENC_PACKED_HEADER_SLICE) {
            LOG_F(INFO,"Support packed slice headers\n");
            configAttrib[configAttribNum].value |= VA_ENC_PACKED_HEADER_SLICE;
        }

        if (tmp & VA_ENC_PACKED_HEADER_MISC) {
            LOG_F(INFO,"Support packed misc headers\n");
            configAttrib[configAttribNum].value |= VA_ENC_PACKED_HEADER_MISC;
        }

        configAttribNum++;
    }

    //TODO: interlacing options - are these required?
    //TODO: RefPicList0 and RefPicList1 - are these required?

    if (attrib[VAConfigAttribEncMaxSlices].value != VA_ATTRIB_NOT_SUPPORTED)
        LOG_F(INFO, "Support %d slices\n", attrib[VAConfigAttribEncMaxSlices].value);

    if(attrib[VAConfigAttribEncSliceStructure].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribEncSliceStructure].value;
        LOG_F(INFO, "Support VAConfigAttribEncSliceStructure\n");

        if (tmp & VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS)
            LOG_F(INFO, "Support VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS\n");
        if (tmp & VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS)
            LOG_F(INFO, "Support VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS\n");
        if (tmp & VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS)
            LOG_F(INFO, "Support VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS\n");
    }

    if (attrib[VAConfigAttribEncMacroblockInfo].value != VA_ATTRIB_NOT_SUPPORTED) {
        LOG_F(INFO, "Support VAConfigAttribEncMacroblockInfo\n");
    }

    return result;
    error:
    return result;
}

bool SnpEncoderVaH264::setupEncode() {
    bool result = true;
    VAStatus vaStatus;
    VASurfaceID tmpSurfaces[2];
    int codedBufSize;

    vaStatus = vaCreateConfig(vaDisplay, VAProfileH264ConstrainedBaseline, selectedEntrypoint,
                              &configAttrib[0], configAttribNum, &configId);
    CHECK_VASTATUS(vaStatus, "vaCreateConfig")

    vaStatus = vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, frameWidthMbAligned, frameHeightMbAligned,
                                &srcSurface[0], SURFACE_NUM, nullptr, 0);
    CHECK_VASTATUS(vaStatus, "vaCreateSurface")

    //reference surfaces
    vaStatus = vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, frameWidthMbAligned, frameHeightMbAligned,
                                &refSurface[0], SURFACE_NUM, nullptr, 0);
    CHECK_VASTATUS(vaStatus, "vaCreateSurface")

    tmpSurfaces[0] = srcSurface[0];
    tmpSurfaces[1] = refSurface[0];

    vaStatus = vaCreateContext(vaDisplay, configId, (int)frameWidthMbAligned, (int)frameHeightMbAligned, VA_PROGRESSIVE,
                               tmpSurfaces, 2, &contextId);
    CHECK_VASTATUS(vaStatus, "vaCreateContext")

    //TODO: how does the formula for codedBufSize arise?
    codedBufSize = (int)(frameWidthMbAligned * frameHeightMbAligned * 400) / (16*16);

    for(int i = 0;i < SURFACE_NUM; i++) {
        vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncCodedBufferType, codedBufSize,
                                  1, nullptr, &codedBuf[i]);
        CHECK_VASTATUS(vaStatus,"vaCreateBuffer")
    }

    return result;
error:
    return result;
}


bool SnpEncoderVaH264::initVaH264Encoder() {
    bool result = true;
    result = initVa();
    ASSERT(result == true);
    result = setupEncode();
    ASSERT(result == true);
    return result;
error:
    return result;
}

bool SnpEncoderVaH264::releaseVaH264Encoder() {
    releaseEncode();
    releaseVa();
}

bool SnpEncoderVaH264::encodeFrameVaH264(const uint8_t *framebuffer, uint32_t len) {
    bool result = true;
    VAStatus vaStatus;
    VAImage surfaceImage;
    void *surfacePtr, *uStartPtr, *vStartPtr;
    uint32_t pitches[3] = {0,0,0};
    VACodedBufferSegment *bufList;
    uint32_t codedSize = 0;

    uint32_t surfaceIndex = encodingFrameNum % SURFACE_NUM;
    VASurfaceID surface = srcSurface[surfaceIndex];

    //upload yuv data to surface
    vaStatus = vaDeriveImage(vaDisplay, surface, &surfaceImage);
    CHECK_VASTATUS(vaStatus, "vaDeriveImage")

    vaStatus = vaMapBuffer(vaDisplay, surfaceImage.buf, &surfacePtr);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer")

    VideoUtil::rgba2Nv12((uint8_t*)surfacePtr, (uint8_t*)framebuffer, (int)width, (int)height,
                         (int)surfaceImage.pitches[0], (int)surfaceImage.pitches[1]);

    vaStatus = vaUnmapBuffer(vaDisplay, surfaceImage.buf);
    CHECK_VASTATUS(vaStatus, "vaUnmapBuffer")

    vaStatus = vaDestroyImage(vaDisplay, surfaceImage.image_id);
    CHECK_VASTATUS(vaStatus, "vaDestroyImage")

    //encode the frame()
    vaStatus = vaBeginPicture(vaDisplay, contextId, surface);
    CHECK_VASTATUS(vaStatus, "vaBeginPicture")

    memset(&seqParam, 0, sizeof(seqParam));
    memset(&picParam, 0, sizeof(picParam));
    memset(&sliceParam, 0, sizeof(sliceParam));

    if (encodingFrameNum % idrPeriod == 0) {
        currentFrameType = FRAME_IDR;
    } else if (encodingFrameNum % iPeriod == 0) {
        currentFrameType = FRAME_I;
    } else {
        currentFrameType = FRAME_P;
    }

    currentCurrPic.picture_id = srcSurface[surfaceIndex];
    currentCurrPic.frame_idx = currentFrameNum;
    currentCurrPic.flags = 0;
    currentCurrPic.TopFieldOrderCnt = 0;
    currentCurrPic.BottomFieldOrderCnt = 0;

    if (currentFrameType == FRAME_IDR) {
        numShortTerm = 0;
        renderSequence();
        renderPackedHeaders();
    }

    renderPicture();
    renderSlice();

    vaStatus = vaEndPicture(vaDisplay, contextId);
    CHECK_VASTATUS(vaStatus, "vaEndPicture")

    vaStatus = vaSyncSurface(vaDisplay, surface);
    CHECK_VASTATUS(vaStatus, "vaSyncSurface")

    vaStatus = vaMapBuffer(vaDisplay, codedBuf[0], (void **)&bufList);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer")

    while(bufList != nullptr) {
        LOG_F(INFO, "encoded size = %d", bufList->size);
        codedSize += bufList->size;
        getOutputPort(0)->onData(getPipeId(), (uint8_t*)bufList->buf, bufList->size, bufList->next == nullptr);
        bufList = (VACodedBufferSegment*)bufList->next;
    }

    LOG_F(INFO, "wrote %d bytes", codedSize);

    vaStatus = vaUnmapBuffer(vaDisplay, codedBuf[0]);
    CHECK_VASTATUS(vaStatus, "vaUnmapBuffer")

    updateReferenceFrames();

    encodingFrameNum++;

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderPackedHeaders() {
    bool result = true;
    VAStatus vaStatus;
    VAEncPackedHeaderParameterBuffer packedHeaderParam = {};
    VABufferID packedHeaderBuf;
    VABufferID packedDataBuf;
    VABufferID spsBuffers[2];
    VABufferID ppsBuffers[2];

    // === SPS ===
    packedHeaderParam.type = VAEncPackedHeaderSequence;
    packedHeaderParam.bit_length = 0;
    packedHeaderParam.has_emulation_bytes = 1;

    vaStatus = vaCreateBuffer(vaDisplay, contextId,
                              VAEncPackedHeaderParameterBufferType,
                              sizeof(packedHeaderParam), 1,
                              &packedHeaderParam, &packedHeaderBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer SPS header")

    vaStatus = vaCreateBuffer(vaDisplay, contextId,
                              VAEncPackedHeaderDataBufferType,
                              0, 1,
                              nullptr, &packedDataBuf);  // driver fills it in
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer SPS data")

    spsBuffers[0] = packedHeaderBuf;
    spsBuffers[1] = packedDataBuf;

    vaStatus = vaRenderPicture(vaDisplay, contextId, spsBuffers, 2);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture SPS")

    // === PPS ===
    packedHeaderParam.type = VAEncPackedHeaderPicture;

    vaStatus = vaCreateBuffer(vaDisplay, contextId,
                              VAEncPackedHeaderParameterBufferType,
                              sizeof(packedHeaderParam), 1,
                              &packedHeaderParam, &packedHeaderBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer PPS header")

    vaStatus = vaCreateBuffer(vaDisplay, contextId,
                              VAEncPackedHeaderDataBufferType,
                              0, 1,
                              nullptr, &packedDataBuf);  // driver fills it in
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer PPS data")

    ppsBuffers[0] = packedHeaderBuf;
    ppsBuffers[1] = packedDataBuf;
    vaStatus = vaRenderPicture(vaDisplay, contextId, ppsBuffers, 2);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture PPS")

    return true;
error:
    return false;
}

void SnpEncoderVaH264::updateReferenceFrames() {
    int i;
    if(currentFrameType != FRAME_B) {
        currentCurrPic.flags = VA_PICTURE_H264_SHORT_TERM_REFERENCE;

        // Shift references to make room at index 0
        for (int i = std::min(numShortTerm, numRefFrames - 1); i > 0; --i) {
            referenceFrames[i] = referenceFrames[i - 1];
        }

        referenceFrames[0] = currentCurrPic;

        if(numShortTerm < numRefFrames) {
            numShortTerm++;
        }
    }

    currentFrameNum++;
    if (currentFrameNum == MaxFrameNum) {
        currentFrameNum = 0;
    }
}

bool SnpEncoderVaH264::renderSequence() {
    bool result = true;
    VAStatus vaStatus;
    VABufferID seqParamBuf;
    VABufferID rcParamBuf;
    VABufferID renderId[2];
    VAEncMiscParameterBuffer *miscParam, *miscParamTmp;
    VAEncMiscParameterRateControl *miscRateCtrl;

    seqParam.level_idc = 41;
    seqParam.picture_width_in_mbs = frameWidthMbAligned / 16;
    seqParam.picture_height_in_mbs = frameHeightMbAligned / 16;
    seqParam.bits_per_second = frameBitrate;

    seqParam.intra_idr_period = idrPeriod;
    seqParam.intra_period = iPeriod;
    seqParam.ip_period = ipPeriod;

    seqParam.max_num_ref_frames = 1;
    seqParam.seq_fields.bits.frame_mbs_only_flag = 1;
    seqParam.time_scale = 900;
    seqParam.num_units_in_tick = 15; /* Tc = num_units_in_tick / time_sacle */
//    seqParam.seq_fields.bits.pic_order_cnt_type = 2;
    seqParam.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 = Log2MaxPicOrderCntLsb - 4;
    seqParam.seq_fields.bits.log2_max_frame_num_minus4 = Log2MaxFrameNum - 4;
    seqParam.seq_fields.bits.frame_mbs_only_flag = 1;
    seqParam.seq_fields.bits.chroma_format_idc = 1;
    seqParam.seq_fields.bits.direct_8x8_inference_flag = 1;

    //TODO: cropping omitted now

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncSequenceParameterBufferType, sizeof(seqParam),
                              1, &seqParam, &seqParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    //rate control
    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncMiscParameterBufferType,
                              sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterRateControl), 1,
                                  nullptr, &rcParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    vaMapBuffer(vaDisplay, rcParamBuf, (void**)&miscParam);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer")

    miscParam->type = VAEncMiscParameterTypeRateControl;
    miscRateCtrl = (VAEncMiscParameterRateControl *)miscParam->data;
    memset(miscRateCtrl, 0, sizeof(*miscRateCtrl));
    miscRateCtrl->bits_per_second = frameBitrate;
    miscRateCtrl->target_percentage = 66;
    miscRateCtrl->window_size = 1000;
    miscRateCtrl->initial_qp = initialQp;
    miscRateCtrl->min_qp = minimalQp;
    miscRateCtrl->basic_unit_size = 0;
    vaUnmapBuffer(vaDisplay, rcParamBuf);

    renderId[0] = seqParamBuf;
    renderId[1] = rcParamBuf;

    vaStatus = vaRenderPicture(vaDisplay, contextId, &renderId[0], 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderPicture() {
    bool result = true;
    VAStatus vaStatus;
    VABufferID picParamBuf;

    picParam.CurrPic = currentCurrPic;
    picParam.num_ref_idx_l0_active_minus1 = 0;
    picParam.num_ref_idx_l1_active_minus1 = 0;

    // Fill the reference frame list
    for (int i = 0; i < numShortTerm; ++i) {
        picParam.ReferenceFrames[i] = referenceFrames[i];
    }

    for (int i = numShortTerm; i < 16; ++i) {
        picParam.ReferenceFrames[i].picture_id = VA_INVALID_SURFACE;
        picParam.ReferenceFrames[i].flags = VA_PICTURE_H264_INVALID;
    }

    picParam.pic_fields.bits.idr_pic_flag = (currentFrameType == FRAME_IDR);
    picParam.pic_fields.bits.reference_pic_flag = 1;
    picParam.pic_fields.bits.entropy_coding_mode_flag = 0; //1 = cabac //TODO: cabac
    picParam.pic_fields.bits.deblocking_filter_control_present_flag = 0; //TODO: filter
    picParam.frame_num = currentFrameNum;
    picParam.coded_buf = codedBuf[0];
    picParam.last_picture = 0;
    picParam.pic_init_qp = initialQp;

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncPictureParameterBufferType, sizeof(picParam),
                              1, &picParam, &picParamBuf);
    CHECK_VASTATUS(vaStatus,"vaCreateBuffer")

    vaStatus = vaRenderPicture(vaDisplay, contextId, &picParamBuf, 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture")

    return result;
    error:
    return result;
}

bool SnpEncoderVaH264::renderSlice() {
    bool result = true;
    VAStatus vaStatus;
    VABufferID sliceParamBuf;

    sliceParam.macroblock_address = 0;

    uint32_t mbWidth  = (width  + 15) / 16;
    uint32_t mbHeight = (height + 15) / 16;

    sliceParam.num_macroblocks = mbWidth * mbHeight;

    if(currentFrameType == FRAME_IDR) {
        //idr-frame
        sliceParam.idr_pic_id++;
    } else if(currentFrameType == FRAME_P || currentFrameType == FRAME_I) {
        for(int i = 0; i < 32; i++) {
            sliceParam.RefPicList0[i].picture_id = VA_INVALID_SURFACE;
            sliceParam.RefPicList0[i].flags = VA_PICTURE_H264_INVALID;
        }
        //exactly one reference frame
        memcpy(sliceParam.RefPicList0, referenceFrames, sizeof(VAPictureH264));
    }
    sliceParam.slice_alpha_c0_offset_div2 = 0;
    sliceParam.slice_beta_offset_div2 = 0;
    sliceParam.direct_spatial_mv_pred_flag = 1;
    sliceParam.slice_type = currentFrameType == FRAME_IDR ? 2 : currentFrameType;
    sliceParam.pic_order_cnt_lsb = encodingFrameNum;

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncSliceParameterBufferType, sizeof(sliceParam),
                              1, &sliceParam, &sliceParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    vaStatus = vaRenderPicture(vaDisplay, contextId, &sliceParamBuf, 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture")

    return result;
    error:
    return result;
}

bool SnpEncoderVaH264::releaseEncode() {
    int i;

    vaDestroySurfaces(vaDisplay, &srcSurface[0], SURFACE_NUM);
    vaDestroySurfaces(vaDisplay, &refSurface[0], SURFACE_NUM);

    for (i = 0; i < SURFACE_NUM; i++)
        vaDestroyBuffer(vaDisplay, codedBuf[i]);

    vaDestroyContext(vaDisplay, contextId);
    vaDestroyConfig(vaDisplay, configId);

    return true;
}

bool SnpEncoderVaH264::releaseVa() {
    vaTerminate(vaDisplay);
    return true;
}
