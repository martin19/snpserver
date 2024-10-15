#include "SnpDecoderAmfH264.h"
#include "public/common/AMFFactory.h"
#include "public/include/components/VideoDecoderUVD.h"
#include "public/include/components/VideoConverter.h"
#include "public/include/core/Plane.h"
#include "public/include/core/Surface.h"
#include "util/VideoUtil.h"


SnpDecoderAmfH264::SnpDecoderAmfH264(const SnpDecoderAmfH264Options &options) : SnpComponent(options, "COMPONENT_DECODER_AMD") {
    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_H264));
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    getInputPort(0)->setOnDataCb(std::bind(&SnpDecoderAmfH264::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

SnpDecoderAmfH264::~SnpDecoderAmfH264() {

}

bool SnpDecoderAmfH264::start() {
    SnpComponent::start();
    if(decoderInit()) {
//        converterInit();
        return true;
    }
    return false;
}

void SnpDecoderAmfH264::stop() {
    SnpComponent::stop();
//    converterDestroy();
    decoderDestroy();
}

bool SnpDecoderAmfH264::decoderInit() {
    AMF_RESULT res = AMF_OK;
    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();

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

    res = amf::AMFContext1Ptr(context)->InitDX11(NULL);
    if (res != AMF_OK) {
        LOG_F(ERROR, "InitDX11(NULL) failed.");
        return false;
    }

    res = g_AMFFactory.GetFactory()->CreateComponent(context, AMFVideoDecoderUVD_H264_AVC, &decoder);
    if (res != AMF_OK) {
        LOG_F(ERROR, "g_AMFFactory.GetFactory()->CreateComponent failed.");
    }

    amf::AMF_SURFACE_FORMAT formatOut = amf::AMF_SURFACE_RGBA;

// our sample H264 parser provides decode order timestamps - change this depend on demuxer
//  decoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_DECODE));

    res = decoder->Init(formatOut, (amf_int32)width, (amf_int32)height);
    if (res != AMF_OK) {
        LOG_F(ERROR, "decoder->Init failed.");
    }

    context->AllocBuffer(amf::AMF_MEMORY_HOST, 1000000, &amfInputBuffer);

    rgbaBuffer = static_cast<uint8_t *>(calloc(width * height * 4, 1));

    return true;
}

void SnpDecoderAmfH264::decode(const uint8_t *data, uint32_t len) {
    AMF_RESULT res = AMF_OK;
    amf::AMFDataPtr amfOutputDataPtr;
    memcpy(amfInputBuffer->GetNative(), data, len);
    res = decoder->SubmitInput(amfInputBuffer);
    if(res == AMF_NEED_MORE_INPUT) {
        //do nothing
        return;
    } else if(res == AMF_INPUT_FULL || res == AMF_DECODER_NO_FREE_SURFACES || res == AMF_REPEAT) {
        // queue is full; sleep, try to get ready surfaces in polling thread and repeat submission
    }

    res = decoder->QueryOutput(&amfOutputDataPtr);
    if (res == AMF_EOF) {
        // Drain complete
    } else if ((res != AMF_OK) && (res != AMF_REPEAT)) {
        // trace possible error message
        // Drain complete
    }

    if (amfOutputDataPtr == nullptr) {
        //no data available at output
        return;
    }

    res = amfOutputDataPtr->Convert(amf::AMF_MEMORY_HOST);
    if(res != AMF_OK) {
        return;
    }

    amf::AMFSurfacePtr surfaceRgba(amfOutputDataPtr);
    amf::AMFPlane* planeRGBA = surfaceRgba->GetPlane(amf::AMF_PLANE_PACKED);

    int planeHPitch = planeRGBA->GetHPitch();

    auto *src = (uint8_t*)planeRGBA->GetNative();
    auto *dst = rgbaBuffer;
    for(int y = 0; y < height; y++) {
        uint8_t* lineSrc = src + y*planeHPitch;
        uint8_t* lineDst = dst + y*(width<<2);
        for(int x = 0; x < width; x++) {
            *lineDst++ = *lineSrc++;
            *lineDst++ = *lineSrc++;
            *lineDst++ = *lineSrc++;
            *lineDst++ = 255;
            lineSrc++;
        }
    }

    SnpPort *outputPort = this->getOutputPort(0);
    outputPort->onData(getPipeId(), (uint8_t*)rgbaBuffer, width * height * 4, true);
}

//bool SnpDecoderAmfH264::converterInit() {
//    AMF_RESULT res = AMF_OK;
//    amf::AMF_SURFACE_FORMAT formatIn = amf::AMF_SURFACE_NV12;
//    amf::AMF_SURFACE_FORMAT formatOut = amf::AMF_SURFACE_RGBA;
//    amf::AMF_MEMORY_TYPE memoryTypeIn = amf::AMF_MEMORY_DX9;
//    amf::AMF_MEMORY_TYPE memoryTypeOut = amf::AMF_MEMORY_DX9;
//    amf::AMF_MEMORY_TYPE memoryTypeCompute = amf::AMF_MEMORY_DX9;
//    res = g_AMFFactory.GetFactory()->CreateComponent(context, AMFVideoConverter, &converter);
//    res = converter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, memoryTypeOut);
//    res = converter->SetProperty(AMF_VIDEO_CONVERTER_COMPUTE_DEVICE, memoryTypeCompute);
//    res = converter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, formatOut);
//    res = converter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, ::AMFConstructSize((amf_int32)width, (amf_int32)height));
//
//    res = converter->Init(formatIn, (amf_int32)width, (amf_int32)height);
//    if(res != AMF_OK) {
//        LOG_F(ERROR, "converter->Init failed.");
//        return false;
//    }
//}

//void SnpDecoderAmfH264::convert(amf::AMFSurfacePtr surfaceIn, amf::AMFSurfacePtr surfaceOut) {
//    AMF_RESULT res = AMF_OK;
//    res = converter->SubmitInput(surfaceIn);
//    if(res != AMF_OK) {
//        LOG_F(ERROR, "AMF converter->SubmitInput failed.");
//        return;
//    }
//
//    amf::AMFDataPtr pData;
//    res = converter->QueryOutput(&pData);
//    if(res != AMF_OK) {
//        LOG_F(ERROR, "AMF converter->QueryOutput failed.");
//        return;
//    }
//
//    amf::AMFSurfacePtr pSurface(pData);
//    surfaceOut = pSurface;
//}

//void SnpDecoderAmfH264::converterDestroy() {
//    converter->Terminate();
//    converter = nullptr;
//}


void SnpDecoderAmfH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    decode(data, len);
}


void SnpDecoderAmfH264::decoderDestroy() {
    // cleanup in this order
    free(rgbaBuffer);
    amfInputBuffer.Release();
    amfInputBuffer = nullptr;
    decoder->Terminate();
    decoder = nullptr;
    context->Terminate();
    context = nullptr; // context is the last
    g_AMFFactory.Terminate();
}
