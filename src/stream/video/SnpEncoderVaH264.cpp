#include <util/assert.h>
#include <iostream>
#include <util/VideoUtil.h>
#include <cstring>
#include "util/loguru.h"
#include "SnpEncoderVaH264.h"
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "va/va_drm.h"
#include "h264/VaBitstream.h"

// https://www.vcodex.com/h264avc-picture-management/

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        fprintf(stderr,"%s:%s (%d) failed with status %d,exit\n", __func__, func, __LINE__, va_status); \
        goto error;                                                        \
    }

#define FRAME_P 0
#define FRAME_B 1
#define FRAME_I 2
#define FRAME_IDR 7

//TODO: what are these?
static unsigned int MaxFrameNum = (2<<16);
static unsigned int MaxPicOrderCntLsb = (2<<4);
static unsigned int Log2MaxFrameNum = 4;
static unsigned int Log2MaxPicOrderCntLsb = 4;

SnpEncoderVaH264::SnpEncoderVaH264(const SnpEncoderVaH264Options &options) : SnpComponent(options) {
    width = options.width;
    height = options.height;
    bpp = options.bytesPerPixel;

    addInputPort(new SnpPort(PORT_TYPE_BOTH));
    addOutputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderVaH264::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    frameWidthMbAligned = (width + 15) & (~15);
    frameHeightMbAligned = (height + 15) & (~15);
    h264Profile = VAProfileH264ConstrainedBaseline;
    constraintSetFlag = 0;
    frameBitrate = 20000000;
    initialQp = 26;
    minimalQp = 26;
    configAttribNum = 0;
    currentFrameNum = 0;
}

SnpEncoderVaH264::~SnpEncoderVaH264() {
    //TODO:
}

void SnpEncoderVaH264::setEnabled(bool enabled) {
    if(enabled) {
        VaH264EncoderInit();
    } else {
        VaH264EncoderDestroy();
    }
    SnpComponent::setEnabled(enabled);
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

    std::cout << "initVa" <<std::endl;
    vaDisplay = vaGetDisplayDRM(getInputPort(0)->deviceFd);
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

    bitstream = new VaBitstream(h264Profile, seqParam, picParam, sliceParam, constraintSetFlag, frameBitrate);

    //upload yuv data to surface
    vaStatus = vaDeriveImage(vaDisplay, srcSurface[0], &surfaceImage);
    CHECK_VASTATUS(vaStatus, "vaDeriveImage");

    vaStatus = vaMapBuffer(vaDisplay, surfaceImage.buf, &surfacePtr);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer");

    //TODO: assuming image is VA_FORMAT_IYUV
//    pitches[0] = surfaceImage.pitches[0];
//    pitches[1] = surfaceImage.pitches[1];
//    pitches[2] = surfaceImage.pitches[3];
//    uStartPtr = (uint8_t*)surfacePtr + surfaceImage.offsets[1];
//    vStartPtr = (uint8_t*)surfacePtr + surfaceImage.offsets[2];

    VideoUtil::rgba2Yuv((uint8_t*)surfacePtr, framebuffer, width, height);

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

//TODO:
//    if (currentFrameNum == 0) {
//        numShortTerm = 0;
//        currentFrameNum = 0;
//        current_IDR_display = current_frame_display;
//    }

    if(currentFrameNum == 0) {
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

    std::cout << "wrote " << codedSize << " bytes" << std::endl;

    vaStatus = vaUnmapBuffer(vaDisplay, codedBuf[0]);
    CHECK_VASTATUS(vaStatus, "vaUnmapBuffer");

    currentFrameNum++;

    delete bitstream;

    return result;
error:
    return result;
}

void SnpEncoderVaH264::VaH264EncoderDestroy() {
    //release_encode();
    //deinit_va();
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

    seqParam.intra_period = 500;
    seqParam.intra_idr_period = 500;
    seqParam.ip_period = 1;

    seqParam.max_num_ref_frames = 1;
    seqParam.seq_fields.bits.frame_mbs_only_flag = 1;
    seqParam.time_scale = 900;
    seqParam.num_units_in_tick = 15; /* Tc = num_units_in_tick / time_sacle */
    seqParam.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 = Log2MaxPicOrderCntLsb - 4;
    seqParam.seq_fields.bits.log2_max_frame_num_minus4 = Log2MaxFrameNum - 4;;
    seqParam.seq_fields.bits.frame_mbs_only_flag = 1;
    seqParam.seq_fields.bits.chroma_format_idc = 1;
    seqParam.seq_fields.bits.direct_8x8_inference_flag = 1;

    //TODO: cropping omitted now

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncSequenceParameterBufferType, sizeof(seqParam),
                              1, &seqParam, &seqParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    //rate control
    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncMiscParameterBufferType,
                              sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterRateControl), 1,
                                  nullptr, &rcParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    vaMapBuffer(vaDisplay, rcParamBuf, (void**)&miscParam);
    CHECK_VASTATUS(vaStatus, "vaMapBuffer");

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

    vaStatus = vaRenderPicture(vaDisplay, contextId, &renderId[0], 2);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

    //TODO: misc_priv_value omitted now.

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
    picParam.CurrPic.TopFieldOrderCnt = 2*currentFrameNum;
    picParam.CurrPic.BottomFieldOrderCnt = 2*currentFrameNum;
    currentCurrPic = picParam.CurrPic;
    picParam.num_ref_idx_l0_active_minus1 = 0;
    picParam.num_ref_idx_l1_active_minus1 = 0;

    for(int i = 0; i < 16; i++) {
        picParam.ReferenceFrames[i].picture_id = VA_INVALID_SURFACE;
        picParam.ReferenceFrames[i].flags = VA_PICTURE_H264_INVALID;
    }

    //setup reference frames
    if(currentFrameNum != 0) {
        picParam.ReferenceFrames[0].picture_id = refSurface[0];
        picParam.ReferenceFrames[0].flags = VA_PICTURE_H264_LONG_TERM_REFERENCE;
    }

    picParam.pic_fields.bits.idr_pic_flag = currentFrameNum == 0; //currentFrameType == FRAME_IDR
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
    if(currentFrameNum == 0) {
        //idr-frame
        sliceParam.idr_pic_id++;
    }
    sliceParam.slice_alpha_c0_offset_div2 = 0;
    sliceParam.slice_beta_offset_div2 = 0;
    sliceParam.direct_spatial_mv_pred_flag = 1;
    sliceParam.slice_type = currentFrameNum == 0 ? 2 : FRAME_P;
    sliceParam.pic_order_cnt_lsb = 0; //(currentFrameDisplay - currentIDRDisplay) % MaxPicOrderCntLsb;

    vaStatus = vaCreateBuffer(vaDisplay, contextId, VAEncSliceParameterBufferType, sizeof(sliceParam),
                              1, &sliceParam, &sliceParamBuf);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    if(h264PackedHeader && (configAttrib[encPackedHeaderIdx].value & VA_ENC_PACKED_HEADER_SLICE)) {
        renderPackedSlice();
    }

    vaStatus = vaRenderPicture(vaDisplay, contextId, &sliceParamBuf, 1);
    CHECK_VASTATUS(vaStatus, "vaRenderPicture");

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

    free(packedsliceBuffer);

    return result;
error:
    return result;
}

