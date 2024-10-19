#include <iostream>
#include <util/VideoUtil.h>
#include <cstring>
#include "util/loguru.h"
#include "SnpEncoderVaH264.h"
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "va/va_win32.h"
#include "d3d12video.h"
#include <cassert>
#include "h264/VaUtils.h"

//TODO: query d3d12 capabilities: (see https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloVAEncode/D3D12HelloVAEncode.cpp)
// https://www.vcodex.com/h264avc-picture-management/

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        LOG_F(ERROR,"%s failed with status %d,exit\n", __func__, (int)va_status); \
        goto error;                                                        \
    }

#define CHECK_RESULT(result, func)                                  \
    if(result != true) {                                            \
        LOG_F(ERROR,"%s failed.\n", __func__);                      \
        goto error;                                                 \
    }

void SnpEncoderVaH264::vaInfoCallback([[maybe_unused]] void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}

void SnpEncoderVaH264::vaErrorCallback([[maybe_unused]] void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}

SnpEncoderVaH264::SnpEncoderVaH264(const SnpEncoderVaH264Options &options) : SnpComponent(options, "COMPONENT_ENCODER_INTEL") {
    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGBA));
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_H264));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    //addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("qp", options.qp));

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderVaH264::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

SnpEncoderVaH264::~SnpEncoderVaH264() {
    //TODO:
}

bool SnpEncoderVaH264::start() {
    SnpComponent::start();

    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();

    initVaEncoder();
    return true;
}

void SnpEncoderVaH264::stop() {
    SnpComponent::stop();
    destroyVa();
}

bool SnpEncoderVaH264::initVaEncoder() {
    yuvBuffer = (uint8_t*)calloc(1, width*height*3/2);
    initVaPipeline();
}

bool SnpEncoderVaH264::initVaPipeline() {
    bool result;
    result = initVaDisplay();
    CHECK_RESULT(result, "initVaDisplay")
    result = ensureVaEncSupport();
    CHECK_RESULT(result, "ensureVaEncSupport")
    result = createVaSurfaces();
    CHECK_RESULT(result, "createVaSurfaces")
    result = initVaEncContext();
    CHECK_RESULT(result, "initVaEncContext")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaDisplay() {
    bool result = true;
    VAStatus vaStatus;
    int majorVer, minorVer;

    vaDisplay = vaGetDisplayWin32(nullptr);
    vaSetInfoCallback(vaDisplay, reinterpret_cast<VAMessageCallback>(&SnpEncoderVaH264::vaInfoCallback), nullptr);
    vaSetErrorCallback(vaDisplay, reinterpret_cast<VAMessageCallback>(&SnpEncoderVaH264::vaErrorCallback), nullptr);

    vaStatus = vaInitialize(vaDisplay, &majorVer, &minorVer);
    CHECK_VASTATUS(vaStatus, "vaInitialize")

    LOG_F(INFO, "va display acquired (version %d.%d)", majorVer, minorVer);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::ensureVaEncSupport() {
    bool result = true;
    VAStatus vaStatus;

    bool supportsH264Enc = false;

    int numEntrypoints = vaMaxNumEntrypoints(vaDisplay);
    std::vector<VAEntrypoint> entrypoints(numEntrypoints);
    vaStatus = vaQueryConfigEntrypoints(vaDisplay,VAProfileH264Main,entrypoints.data(),&numEntrypoints);
    CHECK_VASTATUS(vaStatus, "vaQueryConfigEntrypoints for VAProfileH264Main")

    for (int32_t i = 0; !supportsH264Enc && i < numEntrypoints; i++) {
        if (entrypoints[i] == VAEntrypointEncSlice)
            supportsH264Enc = true;
    }

    if (!supportsH264Enc) {
        LOG_F(ERROR, "VAEntrypointEncSlice not supported for VAProfileH264Main.");
        return false;
    }

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::createVaSurfaces() {
    bool result = true;
    VAStatus vaStatus;

    vaStatus = vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, width, height,
        &vaSurfaceYuv, 1, nullptr, 0);
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaEncContext() {
    bool result = true;
    VAStatus vaStatus;

    vaStatus = vaCreateConfig(vaDisplay,VAProfileH264Main,VAEntrypointEncSlice,
            nullptr,0,&vaEncConfigId);
    CHECK_VASTATUS(vaStatus, "vaCreateConfig")

    vaStatus = vaCreateContext(vaDisplay,vaEncConfigId,(int)width,(int)height,
            VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaEncContextId);
    CHECK_VASTATUS(vaStatus, "vaCreateContext")

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncSequenceParameterBufferType,
            sizeof(VAEncSequenceParameterBufferH264),1, nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncPictureParameterBufferType,
            sizeof(VAEncPictureParameterBufferH264),1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId, VAEncSliceParameterBufferType,
            sizeof(VAEncSliceParameterBufferH264),1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    // Worst case within reason assume same as uncompressed surface
    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncCodedBufferType,
            width * height * 3,1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    return result;
error:
    return result;
}

void SnpEncoderVaH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    if(!isRunning()) return;
    encodeFrameVa(data, len);
}

bool SnpEncoderVaH264::encodeFrameVa(const uint8_t *data, uint32_t len) {
    bool result = true;
    VAStatus status;
    SnpPort *outputPort = this->getOutputPort(0);
    VAImage surfaceImage;
    void *surfacePtr;

//    VaUtils::uploadSurfaceYuv(vaDisplay, vaSurfaceYuv, VA_FOURCC_NV12, (int)width, (int)height,
//                              yuvBuffer, yuvBuffer + width*height, yuvBuffer + width*height);


    status = vaDeriveImage(vaDisplay, vaSurfaceYuv, &surfaceImage);
    CHECK_VASTATUS(status, "vaDeriveImage")

    status = vaMapBuffer(vaDisplay, surfaceImage.buf, &surfacePtr);
    CHECK_VASTATUS(status, "vaMapBuffer")

    VideoUtil::rgba2NV12(yuvBuffer, data, (int) width, (int) height, (int) width, (int) height);

    status = vaUnmapBuffer(vaDisplay, surfaceImage.buf);
    CHECK_VASTATUS(status, "vaUnmapBuffer");

    status = vaDestroyImage(vaDisplay, surfaceImage.image_id);
    CHECK_VASTATUS(status, "vaDestroyImage");


    //encode
    status = vaBeginPicture(vaDisplay, vaEncContextId, vaSurfaceYuv);
    CHECK_VASTATUS(status, "vaBeginPicture")

    // VAEncSequenceParameterBufferH264
    {
        VAEncSequenceParameterBufferH264* pMappedBuf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ], (void**)&pMappedBuf);
        CHECK_VASTATUS(status, "vaMapBuffer")
        memset(pMappedBuf, 0, sizeof(*pMappedBuf));

        // Level 4.1 as per H.264 codec standard
        pMappedBuf->level_idc = 41;

        // 2 * fps_num for 30fps
        pMappedBuf->time_scale = 2 * 30;
        // fps_den
        pMappedBuf->num_units_in_tick = 1;

        pMappedBuf->intra_idr_period = 1;
        pMappedBuf->seq_fields.bits.pic_order_cnt_type = 2;

        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ]);
        CHECK_VASTATUS(status, "vaUnMapBuffer")
    }

    // VAEncPictureParameterBufferH264
    {
        VAEncPictureParameterBufferH264* pMappedBuf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC], (void**)&pMappedBuf);
        CHECK_VASTATUS(status, "vaMapBuffer")
        memset(pMappedBuf, 0, sizeof(*pMappedBuf));

        pMappedBuf->pic_fields.bits.idr_pic_flag = 1;
        // We can use always 0 as each frame is an IDR which resets the GOP
        pMappedBuf->CurrPic.TopFieldOrderCnt = 0;
        pMappedBuf->CurrPic.picture_id = vaSurfaceYuv;
        pMappedBuf->coded_buf = vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT];

        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC]);
        CHECK_VASTATUS(status, "vaUnMapBuffer")
    }

    // VAEncSliceParameterBufferH264
    {
        VAEncSliceParameterBufferH264* pMappedBuf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE], (void**)&pMappedBuf);
        CHECK_VASTATUS(status, "vaMapBuffer")
        memset(pMappedBuf, 0, sizeof(*pMappedBuf));

        pMappedBuf->num_macroblocks = (width / H264_MB_PIXEL_SIZE * height / H264_MB_PIXEL_SIZE);
        pMappedBuf->slice_type = 2; // intra slice
        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE]);
        CHECK_VASTATUS(status, "vaUnMapBuffer")
    }

    // Apply encode, send the first 3 seq, pic, slice buffers
    vaRenderPicture(vaDisplay, vaEncContextId, vaEncPipelineBufferId, 3);

    status = vaEndPicture(vaDisplay, vaEncContextId);
    CHECK_VASTATUS(status, "vaEndPicture")

    // Wait for completion on GPU for the indicated VABuffer/VASurface
    // Attempt vaSyncBuffer if VA driver implements it first
    status = vaSyncBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT], VA_TIMEOUT_INFINITE);
    if (status != VA_STATUS_ERROR_UNIMPLEMENTED) {
        CHECK_VASTATUS(status, "vaSyncBuffer")
    } else {
        // Legacy API call otherwise
        status = vaSyncSurface(vaDisplay, vaSurfaceYuv);
        CHECK_VASTATUS(status, "vaSyncSurface")
    }

    // Flush encoded bitstream to disk
    {
        VACodedBufferSegment *bufList, *buf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT], (void**)&bufList);
        CHECK_VASTATUS(status, "vaMapBuffer");



        for (buf = bufList; buf; buf = (VACodedBufferSegment*) buf->next) {
            outputPort->onData(getPipeId(), reinterpret_cast<uint8_t*>(buf->buf), buf->size, bufList->next == nullptr);
        }

        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT]);
        CHECK_VASTATUS(status, "vaUnMapBuffer");
    }

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::destroyVa() {
    bool result = true;
    VAStatus status;

    destroyVaEnc();

    // Destroy VA Common

    status = vaDestroySurfaces(vaDisplay, vaRenderTargets, FrameCount);
    CHECK_VASTATUS(status, "vaDestroySurfaces")

    status = vaDestroySurfaces(vaDisplay, &vaSurfaceYuv, 1);
    CHECK_VASTATUS(status, "vaDestroySurfaces")

    vaTerminate(vaDisplay);
    CHECK_VASTATUS(status, "vaTerminate")

    free(yuvBuffer);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::destroyVaEnc() {
    bool result = true;
    VAStatus status;

    status = vaDestroyConfig(vaDisplay, vaEncConfigId);
    CHECK_VASTATUS(status, "vaDestroyConfig")

    status = vaDestroyContext(vaDisplay, vaEncContextId);
    CHECK_VASTATUS(status, "vaDestroyContext")

    for (UINT i = 0; i < _countof(vaEncPipelineBufferId); i++) {
        vaDestroyBuffer(vaDisplay, vaEncPipelineBufferId[i]);
        CHECK_VASTATUS(status, "vaDestroyBuffer")
    }

    return result;
error:
    return result;
}