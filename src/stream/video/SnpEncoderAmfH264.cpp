#include <unistd.h>
#include "SnpEncoderAmfH264.h"
#include "public/include/core/Surface.h"
#include "public/include/core/Context.h"
#include "public/common/AMFFactory.h"
#include "public/include/components/VideoEncoderVCE.h"
#include "public/include/components/VideoEncoderHEVC.h"
#include "public/include/components/VideoEncoderAV1.h"
#include "public/common/Thread.h"

static const amf::AMF_SURFACE_FORMAT efcSurfaceFormat[] = {
        amf::AMF_SURFACE_RGBA,
        amf::AMF_SURFACE_BGRA,
        amf::AMF_SURFACE_R10G10B10A2,
        amf::AMF_SURFACE_RGBA_F16 };

static const wchar_t* pCodecNames[] = {
        AMFVideoEncoderVCE_AVC,
        AMFVideoEncoder_HEVC,
        AMFVideoEncoder_AV1
};

SnpEncoderAmfH264::SnpEncoderAmfH264(const SnpEncoderAmfH264Options &options) : SnpComponent(options, "COMPONENT_ENCODER_AMD") {
    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGBA));
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_H264));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("qp", options.qp));

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderAmfH264::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

SnpEncoderAmfH264::~SnpEncoderAmfH264() {

}

bool SnpEncoderAmfH264::start() {
    SnpComponent::start();

    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();
    double fps = getProperty("height")->getValueDouble();

    AMF_RESULT res = AMF_OK;
    res = g_AMFFactory.Init();
    if (res != AMF_OK) {
        LOG_F(ERROR, "AMF Failed to initialize.");
        return false;
    }
    LOG_F(INFO, "AMF initialized.");

    // context
    res = g_AMFFactory.GetFactory()->CreateContext(&context);
    if (res != AMF_OK) {
        LOG_F(ERROR, "AMF Failed to initialize context.");
        return false;
    }

    res = amf::AMFContext1Ptr(context)->InitVulkan(NULL);
    if (res != AMF_OK) {
        LOG_F(ERROR, "InitVulkan(NULL) failed.");
        return false;
    }

    res = g_AMFFactory.GetFactory()->CreateComponent(context, pCodecNames[0], &encoder);
    if (res != AMF_OK) {
        LOG_F(ERROR, "g_AMFFactory.GetFactory()->CreateComponent failed.");
    }

    //TODO: dump a warning if parameter could not be set
//    encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
    encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
    encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
    encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
    encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, 30000000);
    encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, fps);
//4kmode
//    res = pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
//    AMF_RETURN_IF_FAILED(res, L"SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH) failed");
//    res = pEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
//    AMF_RETURN_IF_FAILED(res, L"SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51)");
//    res = pEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
//    AMF_RETURN_IF_FAILED(res, L"SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0)");
    encoder->SetProperty(AMF_VIDEO_ENCODER_LOWLATENCY_MODE, true);
    encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize((int)width, (int)height));

    res = encoder->Init(amf::AMF_SURFACE_RGBA, (int)width, (int)height);
    if (res != AMF_OK) {
        LOG_F(ERROR, "encoder->Init failed.");
    }

    return true;
}

void SnpEncoderAmfH264::stop() {
    SnpComponent::stop();
    surfaceIn = nullptr;
    encoder->Terminate();
    encoder = nullptr;
    context->Terminate();
    context = nullptr;
    g_AMFFactory.Terminate();
}

void SnpEncoderAmfH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    AMF_RESULT res = AMF_OK;
    SnpPort *outputPort = this->getOutputPort(0);

    res = context->AllocSurface(amf::AMF_MEMORY_HOST, amf::AMF_SURFACE_RGBA, (int)width, (int)height, &surfaceIn);
    if (res != AMF_OK) {
        LOG_F(ERROR, "context->AllocSurface failed.");
        return;
    }

    fillRGBASurface(surfaceIn, (uint8_t *) data);
    res = encoder->SubmitInput(surfaceIn);
    if(res == AMF_OK) {
        LOG_F(INFO, "SubmitInput AMF_OK");
    } else if(res == AMF_NEED_MORE_INPUT) {
        LOG_F(INFO, "SubmitInput AMF_NEED_MORE_INPUT");
        // handle full queue, do nothing
    } else if(res == AMF_INPUT_FULL || res == AMF_DECODER_NO_FREE_SURFACES) {
        LOG_F(INFO, "SubmitInput AMF_INPUT_FULL || AMF_DECODER_NO_FREE_SURFACES");
        //amf_sleep(1); // input queue is full: wait, poll and submit again
        //if input queue is full, drop the frame for now.
    } else {
        LOG_F(ERROR, "SubmitInput failed.");
        surfaceIn = nullptr;
    }

    //TODO: following code is inside a pollingthread, keep it synchronous for now.
    amf::AMFDataPtr pData;
    res = encoder->QueryOutput(&pData);
    if (res == AMF_EOF) {
        // Drain complete
    } else if ((res != AMF_OK) && (res != AMF_REPEAT)) {
        // trace possible error message
        // Drain complete
    }
    if (pData != nullptr) {
        amf::AMFBufferPtr pBuffer(pData);
        outputPort->onData(getPipeId(), (uint8_t*)pBuffer->GetNative(), pBuffer->GetSize(), true);
    }
}

void SnpEncoderAmfH264::fillRGBASurface(amf::AMFSurface* pSurface, uint8_t *rgba) {
    //amf::AMFComputePtr pCompute;
    //context->GetCompute(amf::AMF_MEMORY_VULKAN, &pCompute);
    amf::AMFPlane* pPlane = pSurface->GetPlaneAt(0);
    amf_int32 w = pPlane->GetWidth();
    amf_int32 h = pPlane->GetHeight();
    amf_int32 line = pPlane->GetHPitch();
    auto* pData = (amf_uint8*)pPlane->GetNative();

    for (amf_int32 y = 0; y < h; y++) {
        amf_uint8* pDataLine = pData + y * line;
        uint8_t* src = rgba + y * w * 4;
        for (amf_int32 x = 0; x < w; x++) {
            *pDataLine++ = *src++;
            *pDataLine++ = *src++;
            *pDataLine++ = *src++;
            *pDataLine++ = *src++;
        }
    }
}

