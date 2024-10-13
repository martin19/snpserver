#include "SnpDecoderAmfH264.h"
#include "public/common/AMFFactory.h"
#include "public/include/components/VideoDecoderUVD.h"
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
    return decoderInit();
}

void SnpDecoderAmfH264::stop() {
    SnpComponent::stop();
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

    res = amf::AMFContext1Ptr(context)->InitDX9(NULL);
    if (res != AMF_OK) {
        LOG_F(ERROR, "InitDX9(NULL) failed.");
        return false;
    }

    res = g_AMFFactory.GetFactory()->CreateComponent(context, AMFVideoDecoderUVD_H264_AVC, &decoder);
    if (res != AMF_OK) {
        LOG_F(ERROR, "g_AMFFactory.GetFactory()->CreateComponent failed.");
    }

//    amf::AMF_SURFACE_FORMAT formatOut = amf::AMF_SURFACE_RGBA;
    amf::AMF_SURFACE_FORMAT formatOut = amf::AMF_SURFACE_NV12;

    //TODO: dump a warning if parameter could not be set
//    decoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_DECODE)); // our sample H264 parser provides decode order timestamps - change this depend on demuxer

    res = decoder->Init(formatOut, (amf_int32)width, (amf_int32)height);
    if (res != AMF_OK) {
        LOG_F(ERROR, "decoder->Init failed.");
    }

    context->AllocBuffer(amf::AMF_MEMORY_HOST, 1000000, &amfInputBuffer);

    rgbaBuffer = static_cast<uint8_t *>(calloc(width * height * 4, 1));

    return true;
}

void SnpDecoderAmfH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    AMF_RESULT res = AMF_OK;
    amf::AMFDataPtr amfOutputDataPtr;
    memcpy(amfInputBuffer->GetNative(), data, len);
    res = decoder->SubmitInput(amfInputBuffer);
    if(res == AMF_NEED_MORE_INPUT) {
        //do nothing
    } else if(res == AMF_INPUT_FULL || res == AMF_DECODER_NO_FREE_SURFACES || res == AMF_REPEAT) {
        // queue is full; sleep, try to get ready surfaces  in polling thread and repeat submission
        //TODO:
    } else {
        // submission succeeded. read new buffer from parser
    }

    res = decoder->QueryOutput(&amfOutputDataPtr);
    if (res == AMF_EOF) {
        // Drain complete
    } else if ((res != AMF_OK) && (res != AMF_REPEAT)) {
        // trace possible error message
        // Drain complete
    }
    if (amfOutputDataPtr == nullptr) {
        return;
    }

    res = amfOutputDataPtr->Convert(amf::AMF_MEMORY_HOST);
    if(res != AMF_OK) {
        return;
    }
    amf::AMFSurfacePtr pSurface(amfOutputDataPtr);
    int planesCount = pSurface->GetPlanesCount();
    amf::AMF_SURFACE_FORMAT format = pSurface->GetFormat();
    //    amf::AMFPlane* plane = pSurface->GetPlaneAt(0);
    amf::AMFPlane* planeY = pSurface->GetPlane(amf::AMF_PLANE_Y);
    amf::AMFPlane* planeUV = pSurface->GetPlane(amf::AMF_PLANE_UV);


    int planeYWidth = planeY->GetWidth();
    int planeYHeight = planeY->GetHeight();
    int planeYBpp = planeY->GetPixelSizeInBytes();
    int planeYHPitch = planeY->GetHPitch();
    int planeYVPitch = planeY->GetVPitch();
    amf::AMF_PLANE_TYPE planeYType = planeY->GetType();

    int planeUVWidth = planeUV->GetWidth();
    int planeUVHeight = planeUV->GetHeight();
    int planeUVBpp = planeUV->GetPixelSizeInBytes();
    int planeUVHPitch = planeUV->GetHPitch();
    int planeUVVPitch = planeUV->GetVPitch();
    amf::AMF_PLANE_TYPE planeUVType = planeUV->GetType();

    //TODO: alloc rgba buffer
    VideoUtil::nv12ToRgba(rgbaBuffer, (uint8_t*)planeY->GetNative(), (uint8_t*)planeUV->GetNative(), width, height, planeYHPitch, planeUVHPitch);

    SnpPort *outputPort = this->getOutputPort(0);
    outputPort->onData(getPipeId(), (uint8_t*)rgbaBuffer, width * height * 4, true);
    //outputPort->onData(getPipeId(), (uint8_t*)planeY->GetNative(), width * height, true);
}

void SnpDecoderAmfH264::decoderDestroy() {
    // cleanup in this order
    amfInputBuffer = nullptr;
    decoder->Terminate();
    decoder = nullptr;
    context->Terminate();
    context = nullptr; // context is the last
    g_AMFFactory.Terminate();
}
