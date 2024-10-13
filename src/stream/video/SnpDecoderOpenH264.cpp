#include "SnpDecoderOpenH264.h"
#include "util/assert.h"
#ifdef _WIN32
#include "windows.h"
#include "libloaderapi.h"
#include "util/VideoUtil.h"
#include "util/loguru.h"

#endif


SnpDecoderOpenH264::SnpDecoderOpenH264(const SnpDecoderOpenH264Options &options) : SnpComponent(options, "COMPONENT_DECODER_OPENH264") {
#ifdef _WIN32
    HMODULE hDLL = LoadLibraryA("openh264-2.1.1-win64.dll");
    if(hDLL == nullptr) {
        LOG_F(ERROR, "could not load openh264-2.1.1-win64.dll.");
        exit(-1);
    }

    openH264Api.welsCreateDecoderFunc = (WelsCreateDecoderFunc)GetProcAddress(hDLL, "WelsCreateDecoder");
    openH264Api.welsDestroyDecoderFunc = (WelsDestroyDecoderFunc) GetProcAddress(hDLL, "WelsDestroyDecoder");
#else
    openH264Api.welsCreateSvcEncoderFunc = WelsCreateSVCEncoder;
    openH264Api.welsDestroySvcEncoderFunc = WelsDestroySVGEncoder;
#endif //_WIN32

    decoder = nullptr;
    yuvBuffer[0] = nullptr;
    yuvBuffer[1] = nullptr;
    yuvBuffer[2] = nullptr;
    rgbBuffer = nullptr;

    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_H264));
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    addProperty(new SnpProperty("qp", options.qp));
    getInputPort(0)->setOnDataCb(std::bind(&SnpDecoderOpenH264::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

SnpDecoderOpenH264::~SnpDecoderOpenH264() {
    openH264DecoderDestroy();
}

bool SnpDecoderOpenH264::start() {
    SnpComponent::start();
    openH264DecoderInit();
    return true;
}

void SnpDecoderOpenH264::stop() {
    SnpComponent::stop();
    openH264DecoderDestroy();
}

void SnpDecoderOpenH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    if(!isRunning()) return;
    this->openH264DecoderDecode(data, len);
}

bool SnpDecoderOpenH264::openH264DecoderInit() {
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();
    uint32_t bpp = getProperty("qp")->getValueUint32();

    SDecodingParam sDecodingParam = {0};
    bool result = true;
    int res;
    res = openH264Api.welsCreateDecoderFunc(&decoder);
    ASSERT(res == 0);
    ASSERT(decoder != nullptr);

    sDecodingParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
    sDecodingParam.bParseOnly = false;

    res = decoder->Initialize(&sDecodingParam);
    ASSERT(res == 0);

    yuvBuffer[0] = (uint8_t*)calloc(width*height,1);
    yuvBuffer[1] = (uint8_t*)calloc(width*height/4,1);
    yuvBuffer[2] = (uint8_t*)calloc(width*height/4,1);
    rgbBuffer = (uint8_t*)calloc(width * height * bpp, 1);

    return result;
error:
    return result;
}

bool SnpDecoderOpenH264::openH264DecoderDecode(const uint8_t *srcBuffer, int srcLen) {
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();
    uint32_t bpp = 4;

    bool result = true;
    int res;
    SnpPort *outputPort = this->getOutputPort(0);

    LOG_F(INFO, "received data len=%d", srcLen);
    SBufferInfo sBufferInfo;
    //res = decoder->DecodeFrame2(srcBuffer, srcLen, yuvBuffer, &sBufferInfo);
    res = decoder->DecodeFrameNoDelay(srcBuffer, srcLen, yuvBuffer, &sBufferInfo);
    if(sBufferInfo.iBufferStatus != 1) {
        LOG_F(WARNING, "DecodeFrame2 failed. decoding-state=%d", res);
        return false;
    }
    LOG_F(INFO, "DecodeFrame2 success.");

    VideoUtil::yuv420ToRgba(rgbBuffer, yuvBuffer, width, height,
                            sBufferInfo.UsrData.sSystemBuffer.iStride[0],
                            sBufferInfo.UsrData.sSystemBuffer.iStride[1]);

    outputPort->onData(getPipeId(), rgbBuffer, width * height * bpp, true);

    return result;
error:
    return result;
}

void SnpDecoderOpenH264::openH264DecoderDestroy() {
    if(decoder) {
        decoder->Uninitialize();
        openH264Api.welsDestroyDecoderFunc(decoder);
    }

    free(yuvBuffer[0]);
    free(yuvBuffer[1]);
    free(yuvBuffer[2]);
    free(rgbBuffer);
}
