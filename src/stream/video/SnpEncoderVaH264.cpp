#include <util/assert.h>
#include <iostream>
#include <util/VideoUtil.h>
#include <cstring>
#include "util/loguru.h"
#include "SnpEncoderVaH264.h"
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "va/va_drm.h"
#include "va/va_x11.h"
#include "h264/VaBitstream.h"

// https://www.vcodex.com/h264avc-picture-management/

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        LOG_F(ERROR,"%s failed with status %d,exit\n", __func__, va_status); \
        goto error;                                                        \
    }

#define FRAME_P 0
#define FRAME_B 1
#define FRAME_I 2
#define FRAME_IDR 7

//TODO: what are these?
static unsigned int MaxFrameNum = (2<<4);
static unsigned int MaxPicOrderCntLsb = (2<<16);
static unsigned int Log2MaxFrameNum = 4;
static unsigned int Log2MaxPicOrderCntLsb = 4;

SnpEncoderVaH264::SnpEncoderVaH264(const SnpEncoderVaH264Options &options) : SnpComponent(options, "encoderVaH264") {
    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO));
    addOutputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderVaH264::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    h264Profile = VAProfileH264ConstrainedBaseline;
    constraintSetFlag = 0;
    frameBitrate = 30000000;
    initialQp = 30;
    minimalQp = 30;

    iFramePeriod = 60;
    idrFramePeriod = 0;
    ipPeriod = 1;

    configAttribNum = 0;
    currentFrameNum = 0;
    currentFrameType = FRAME_IDR;
    encodingFrameNum = 0;

    numShortTerm = 0;
}

SnpEncoderVaH264::~SnpEncoderVaH264() {
    //TODO:
}

void SnpEncoderVaH264::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
    if(enabled) {
        auto *format = (StreamFormatVideo*)getInputPort(0)->sourcePort->getFormat();
        width = format->width;
        height = format->height;
        frameWidthMbAligned = (width + 15) & (~15);
        frameHeightMbAligned = (height + 15) & (~15);

        VaH264EncoderInit();
    } else {
        VaH264EncoderDestroy();
    }
}

void SnpEncoderVaH264::onInputData(const uint8_t *data, uint32_t len, bool complete) {
    if(!isEnabled()) return;
    this->VaH264EncoderEncode(data, len);
}

bool SnpEncoderVaH264::initVa() {
    bool result = true;
    VAEntrypoint *entryPoints;
    VAStatus vaStatus;
    int maxNumEntryPoints;
    int numEntryPoints;
    bool supportsEncoding = false;

    int majorVer, minorVer;

    std::cout << "initVa with drm device fd="<< getInputPort(0)->sourcePort->deviceFd <<std::endl;
//    vaDisplay = vaGetDisplayDRM(getInputPort(0)->sourcePort->deviceFd);
    vaDisplay = vaGetDisplay(XOpenDisplay(":0.0"));
    vaStatus = vaInitialize(vaDisplay, &majorVer, &minorVer);
    CHECK_VASTATUS(vaStatus, "vaInitialize");

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
    CHECK_VASTATUS(vaStatus, "vaGetConfigAttributes");

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
        LOG_F(INFO,"Support rate control mode (0x%x):", tmp);

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

        h264PackedHeader = true;
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

        encPackedHeaderIdx = configAttribNum;
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
    CHECK_VASTATUS(vaStatus, "vaCreateConfig");

    vaStatus = vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, frameWidthMbAligned, frameHeightMbAligned,
                                &srcSurface[0], SURFACE_NUM, nullptr, 0);
    CHECK_VASTATUS(vaStatus, "vaCreateSurface");

    //reference surfaces
    vaStatus = vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, frameWidthMbAligned, frameHeightMbAligned,
                                &refSurface[0], SURFACE_NUM, nullptr, 0);
    CHECK_VASTATUS(vaStatus, "vaCreateSurface");

    tmpSurfaces[0] = srcSurface[0];
    tmpSurfaces[1] = refSurface[0];

    vaStatus = vaCreateContext(vaDisplay, configId, frameWidthMbAligned, frameHeightMbAligned, VA_PROGRESSIVE,
                               tmpSurfaces, 2, &contextId);
    CHECK_VASTATUS(vaStatus, "vaCreateContext");

    //TODO: how does the formula for codedBufSize arise?
    codedBufSize = (frameWidthMbAligned * frameHeightMbAligned * 400) / (16*16);

    for(int i = 0;i < SURFACE_NUM; i++) {
        vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncCodedBufferType, codedBufSize,
                                  1, nullptr, &codedBuf[i]);
        CHECK_VASTATUS(vaStatus,"vaCreateBuffer");
    }

    return result;
error:
    return result;
}


bool SnpEncoderVaH264::VaH264EncoderInit() {
    bool result = true;
    result = initVa();
    ASSERT(result == true);
    result = setupEncode();
    ASSERT(result == true);
    return result;
error:
    return result;
}

bool SnpEncoderVaH264::VaH264EncoderEncode(const uint8_t *framebuffer, uint32_t len) {
    bool result = true;
    VAStatus vaStatus;
    VAImage surfaceImage;
    void *surfacePtr, *uStartPtr, *vStartPtr;
    uint32_t pitches[3] = {0,0,0};
    VACodedBufferSegment *bufList;
    uint32_t codedSize = 0;

//    std::cout << encodingFrameNum << std::endl;

    bitstream = new VaBitstream(h264Profile, seqParam, picParam, sliceParam, constraintSetFlag, frameBitrate);

    //upload yuv data to surface
    vaStatus = vaDeriveImage(vaDisplay, srcSurface[0], &surfaceImage);
    CHECK_VASTATUS(vaStatus, "vaDeriveImage");

//    std::cout << surfaceImage.format.fourcc << std::endl;

    vaStatus = vaMapBuffer(vaDisplay, surfaceImage.buf, &surfacePtr);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer");

    VideoUtil::rgba2NV1((uint8_t*)surfacePtr, framebuffer, width, height, frameWidthMbAligned, frameHeightMbAligned);

    vaStatus = vaUnmapBuffer(vaDisplay, surfaceImage.buf);
    CHECK_VASTATUS(vaStatus, "vaUnmapBuffer");

    vaStatus = vaDestroyImage(vaDisplay, surfaceImage.image_id);
    CHECK_VASTATUS(vaStatus, "vaDestroyImage");

    //encode the frame()
    vaStatus = vaBeginPicture(vaDisplay, contextId, srcSurface[0]);
    CHECK_VASTATUS(vaStatus, "vaBeginPicture");

    memset(&seqParam, 0, sizeof(seqParam));
    memset(&picParam, 0, sizeof(picParam));
    memset(&sliceParam, 0, sizeof(sliceParam));

    if(encodingFrameNum == 0) {
        currentFrameType = FRAME_IDR;
    } else if(iFramePeriod != 0 && (encodingFrameNum-1) % iFramePeriod == 0) {
        currentFrameType = FRAME_I;
    } else {
        currentFrameType = FRAME_P;
    }

    if (currentFrameType == FRAME_IDR) {
        numShortTerm = 0;
//        currentFrameNum = 0;
//        current_IDR_display = current_frame_display;
    }

    if(currentFrameType == FRAME_IDR) {
        renderSequence();
        renderPicture();
        if(h264PackedHeader) {
            renderPackedSequence();
            renderPackedPicture();
        }
    } else {
        renderPicture();
    }
    renderSlice();

    vaStatus = vaEndPicture(vaDisplay, contextId);
    CHECK_VASTATUS(vaStatus, "vaEndPicture");

    vaStatus = vaSyncSurface(vaDisplay, refSurface[0]);
    CHECK_VASTATUS(vaStatus, "vaSyncSurface");

    vaStatus = vaMapBuffer(vaDisplay, codedBuf[0], (void **)&bufList);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer");

    while(bufList != nullptr) {
        codedSize += bufList->size;
        getOutputPort(0)->onData((uint8_t*)bufList->buf, bufList->size, bufList->next == nullptr);
        bufList = (VACodedBufferSegment*)bufList->next;
    }

//    std::cout << "wrote " << codedSize << " bytes" << std::endl;

    vaStatus = vaUnmapBuffer(vaDisplay, codedBuf[0]);
    CHECK_VASTATUS(vaStatus, "vaUnmapBuffer");

    updateReferenceFrames();

    delete bitstream;

    encodingFrameNum++;

    return result;
error:
    return result;
}

void SnpEncoderVaH264::updateReferenceFrames() {
    int i;

//    currentCurrPic.flags = VA_PICTURE_H264_SHORT_TERM_REFERENCE;
//    numShortTerm++;
//    if (numShortTerm > numRefFrames)
//        numShortTerm = numRefFrames;
//    for (i=numShortTerm-1; i>0; i--)
//        referenceFrames[i] = referenceFrames[i-1];
//    referenceFrames[0] = currentCurrPic;

    if (currentFrameType != FRAME_B)
        currentFrameNum++;
    if (currentFrameNum == MaxFrameNum)
        currentFrameNum = 0;
}

void SnpEncoderVaH264::VaH264EncoderDestroy() {

    //release_encode
    vaDestroySurfaces(vaDisplay, &srcSurface[0], SURFACE_NUM);
    vaDestroySurfaces(vaDisplay, &refSurface[0], SURFACE_NUM);
    for(int i = 0; i < SURFACE_NUM; i++) {
        vaDestroyBuffer(vaDisplay, codedBuf[0]);
    }
    vaDestroyContext(vaDisplay, contextId);
    vaDestroyConfig(vaDisplay, configId);

    //deinit_va();
    vaTerminate(vaDisplay);
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

    seqParam.intra_period = iFramePeriod;
    seqParam.intra_idr_period = idrFramePeriod;
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

    if (width != frameWidthMbAligned ||
        height != frameHeightMbAligned) {
        seqParam.frame_cropping_flag = 1;
        seqParam.frame_crop_left_offset = 0;
        seqParam.frame_crop_right_offset = frameWidthMbAligned - width;
        seqParam.frame_crop_top_offset = 0;
        seqParam.frame_crop_bottom_offset = frameHeightMbAligned - height;
    }

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncSequenceParameterBufferType, sizeof(seqParam),
                              1, &seqParam, &seqParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    //rate control
//    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncMiscParameterBufferType,
//                              sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterRateControl), 1,
//                                  nullptr, &rcParamBuf);
//    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");
//
//    vaMapBuffer(vaDisplay, rcParamBuf, (void**)&miscParam);
//    CHECK_VASTATUS(vaStatus, "vaMapBuffer");
//
//    miscParam->type = VAEncMiscParameterTypeRateControl;
//    miscRateCtrl = (VAEncMiscParameterRateControl *)miscParam->data;
//    memset(miscRateCtrl, 0, sizeof(*miscRateCtrl));
//    miscRateCtrl->bits_per_second = frameBitrate;
//    miscRateCtrl->target_percentage = 66;
//    miscRateCtrl->window_size = 1000;
//    miscRateCtrl->initial_qp = initialQp;
//    miscRateCtrl->min_qp = minimalQp;
//    miscRateCtrl->basic_unit_size = 0;
//    vaUnmapBuffer(vaDisplay, rcParamBuf);

    renderId[0] = seqParamBuf;
//    renderId[1] = rcParamBuf;

    vaStatus = vaRenderPicture(vaDisplay, contextId, &renderId[0], 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

    //TODO: misc_priv_value omitted now.

    vaDestroyBuffer(vaDisplay, seqParamBuf);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderPicture() {
    bool result = true;
    VAStatus vaStatus;
    VABufferID picParamBuf;

    picParam.CurrPic.picture_id = refSurface[0];
    picParam.CurrPic.frame_idx = currentFrameNum;
    picParam.CurrPic.flags = 0;
    picParam.CurrPic.TopFieldOrderCnt = 0;
    picParam.CurrPic.BottomFieldOrderCnt = 0;
    currentCurrPic = picParam.CurrPic;
    picParam.num_ref_idx_l0_active_minus1 = 1;
    picParam.num_ref_idx_l1_active_minus1 = 0;

    for(int i = 0; i < 16; i++) {
        picParam.ReferenceFrames[i].picture_id = VA_INVALID_SURFACE;
        picParam.ReferenceFrames[i].flags = VA_PICTURE_H264_INVALID;
    }

    //setup reference frames
    if(currentFrameType != FRAME_IDR) {
        picParam.ReferenceFrames[0].picture_id = refSurface[0];
        picParam.ReferenceFrames[0].flags = VA_PICTURE_H264_SHORT_TERM_REFERENCE;
    }

    picParam.pic_fields.bits.idr_pic_flag = currentFrameType == FRAME_IDR;
    picParam.pic_fields.bits.reference_pic_flag = 1;
    picParam.pic_fields.bits.entropy_coding_mode_flag = 0; //1 = cabac //TODO: cabac
    picParam.pic_fields.bits.deblocking_filter_control_present_flag = 0; //TODO: filter
    picParam.frame_num = currentFrameNum;
    picParam.coded_buf = codedBuf[0];
    picParam.last_picture = 0;
    picParam.pic_init_qp = initialQp;

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncPictureParameterBufferType, sizeof(picParam),
                              1, &picParam, &picParamBuf);
    CHECK_VASTATUS(vaStatus,"vaCreateBuffer");

    vaStatus = vaRenderPicture(vaDisplay, contextId, &picParamBuf, 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

    vaStatus = vaDestroyBuffer(vaDisplay, picParamBuf);
    CHECK_VASTATUS(vaStatus,"vaDestroyBuffer");

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderSlice() {
    bool result = true;
    VAStatus vaStatus;
    VABufferID sliceParamBuf;

    sliceParam.macroblock_address = 0;
    sliceParam.num_macroblocks = frameWidthMbAligned * frameHeightMbAligned/(16*16);
    if(currentFrameType == FRAME_IDR) {
        //idr-frame
        sliceParam.idr_pic_id++;
    } else if(currentFrameType == FRAME_P || currentFrameType == FRAME_I) {
        for(int i = 0; i < 32;i++) {
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
    sliceParam.pic_order_cnt_lsb = 0; //(currentFrameDisplay - currentIDRDisplay) % MaxPicOrderCntLsb;

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncSliceParameterBufferType, sizeof(sliceParam),
                              1, &sliceParam, &sliceParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    if(h264PackedHeader && (configAttrib[encPackedHeaderIdx].value & VA_ENC_PACKED_HEADER_SLICE)) {
        renderPackedSlice();
    }

    vaStatus = vaRenderPicture(vaDisplay, contextId, &sliceParamBuf, 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

    vaStatus = vaDestroyBuffer(vaDisplay, sliceParamBuf);
    CHECK_VASTATUS(vaStatus,"vaDestroyBuffer");

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderPackedSequence() {
    bool result = true;

    VAStatus vaStatus;
    VAEncPackedHeaderParameterBuffer headerParameterBuffer;
    VABufferID packedseqParaBufid, packedseqDataBufid, renderId[2];
    unsigned int lengthInBits;
    unsigned char *packedseqBuffer = nullptr;

    lengthInBits = bitstream->build_packed_seq_buffer(&packedseqBuffer);

    headerParameterBuffer.type = VAEncPackedHeaderSequence;

    headerParameterBuffer.bit_length = lengthInBits; /*length_in_bits*/
    headerParameterBuffer.has_emulation_bytes = 0;
    vaStatus = vaCreateBuffer(vaDisplay,
                              contextId,
                              VAEncPackedHeaderParameterBufferType,
                              sizeof(headerParameterBuffer), 1, &headerParameterBuffer,
                              &packedseqParaBufid);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    vaStatus = vaCreateBuffer(vaDisplay,
                              contextId,
                              VAEncPackedHeaderDataBufferType,
                               (lengthInBits + 7) / 8, 1, packedseqBuffer,
                              &packedseqDataBufid);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    renderId[0] = packedseqParaBufid;
    renderId[1] = packedseqDataBufid;
    vaStatus = vaRenderPicture(vaDisplay, contextId, renderId, 2);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");


    vaStatus = vaDestroyBuffer(vaDisplay, packedseqParaBufid);
    CHECK_VASTATUS(vaStatus, "vaDestroyBuffer");

    vaStatus = vaDestroyBuffer(vaDisplay, packedseqDataBufid);
    CHECK_VASTATUS(vaStatus, "vaDestroyBuffer");

    free(packedseqBuffer);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderPackedPicture() {
    bool result = true;

    VAStatus vaStatus;
    VAEncPackedHeaderParameterBuffer headerParameterBuffer;
    VABufferID packedpicParaBufid, packedpicDataBufid, render_id[2];
    unsigned int lengthInBits;
    unsigned char *packedpicBuffer = nullptr;

    lengthInBits = bitstream->build_packed_pic_buffer(&packedpicBuffer);
    headerParameterBuffer.type = VAEncPackedHeaderPicture;
    headerParameterBuffer.bit_length = lengthInBits;
    headerParameterBuffer.has_emulation_bytes = 0;

    vaStatus = vaCreateBuffer(vaDisplay,
                              contextId,
                              VAEncPackedHeaderParameterBufferType,
                              sizeof(headerParameterBuffer), 1, &headerParameterBuffer,
                              &packedpicParaBufid);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    vaStatus = vaCreateBuffer(vaDisplay,
                              contextId,
                              VAEncPackedHeaderDataBufferType,
                               (lengthInBits + 7) / 8, 1, packedpicBuffer,
                              &packedpicDataBufid);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    render_id[0] = packedpicParaBufid;
    render_id[1] = packedpicDataBufid;
    vaStatus = vaRenderPicture(vaDisplay, contextId, render_id, 2);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

    vaStatus = vaDestroyBuffer(vaDisplay, packedpicParaBufid);
    CHECK_VASTATUS(vaStatus, "vaDestroyBuffer");

    vaStatus = vaDestroyBuffer(vaDisplay, packedpicDataBufid);
    CHECK_VASTATUS(vaStatus, "vaDestroyBuffer");

    free(packedpicBuffer);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::renderPackedSlice() {
    bool result = true;

    VAStatus vaStatus;
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedsliceParaBufid, packedsliceDataBufid, renderId[2];
    unsigned int lengthInBits;
    unsigned char *packedsliceBuffer = nullptr;

    lengthInBits = bitstream->build_packed_slice_buffer(&packedsliceBuffer);
    packedheader_param_buffer.type = VAEncPackedHeaderSlice;
    packedheader_param_buffer.bit_length = lengthInBits;
    packedheader_param_buffer.has_emulation_bytes = 0;

    vaStatus = vaCreateBuffer(vaDisplay,
                              contextId,
                              VAEncPackedHeaderParameterBufferType,
                              sizeof(packedheader_param_buffer), 1, &packedheader_param_buffer,
                              &packedsliceParaBufid);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    vaStatus = vaCreateBuffer(vaDisplay,
                              contextId,
                              VAEncPackedHeaderDataBufferType,
                              (lengthInBits + 7) / 8, 1, packedsliceBuffer,
                              &packedsliceDataBufid);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    renderId[0] = packedsliceParaBufid;
    renderId[1] = packedsliceDataBufid;
    vaStatus = vaRenderPicture(vaDisplay, contextId, renderId, 2);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

    vaStatus = vaDestroyBuffer(vaDisplay, packedsliceParaBufid);
    CHECK_VASTATUS(vaStatus, "vaDestroyBuffer");

    vaStatus = vaDestroyBuffer(vaDisplay, packedsliceDataBufid);
    CHECK_VASTATUS(vaStatus, "vaDestroyBuffer");

    free(packedsliceBuffer);

    return result;
error:
    return result;
}

