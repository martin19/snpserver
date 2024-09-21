#include "SnpDecoderOpenH264.h"
#include "util/assert.h"
#ifdef _WIN32
#include "windows.h"
#include "libloaderapi.h"
#include "util/VideoUtil.h"
#include "util/loguru.h"

#endif


SnpDecoderOpenH264::SnpDecoderOpenH264(const SnpDecoderOpenH264Options &options) : SnpComponent(options, "decoderOpenH264") {
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

    addInputPort(new SnpPort());
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO));

    getInputPort(0)->setOnDataCb(std::bind(&SnpDecoderOpenH264::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpDecoderOpenH264::~SnpDecoderOpenH264() {
    openH264DecoderDestroy();
}

void SnpDecoderOpenH264::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
    if(enabled) {
        //auto *format = (StreamFormatVideo*)getOutputPort(0)->sourcePort->getFormat();
//        width = format->width;
//        height = format->height;
        width = 1920;
        height = 1080;
        bpp = 4;
        openH264DecoderInit();
    } else {
        openH264DecoderDestroy();
    }
}

void SnpDecoderOpenH264::onInputData(const uint8_t *data, uint32_t len, bool complete) {
    if(!isRunning()) return;
    this->openH264DecoderDecode(data, len);
}

bool SnpDecoderOpenH264::openH264DecoderInit() {
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
    bool result = true;
    int res;
    SnpPort *outputPort = this->getOutputPort(0);

    LOG_F(INFO, "received data len=%d", srcLen);
    SBufferInfo sBufferInfo;
    decoder->DecodeFrame2(srcBuffer, srcLen, yuvBuffer, &sBufferInfo);
    if(sBufferInfo.iBufferStatus != 1) {
        LOG_F(WARNING, "DecodeFrame2 failed.");
        return false;
    }
    LOG_F(INFO, "DecodeFrame2 success.");

    VideoUtil::yuv420ToRgba(rgbBuffer, yuvBuffer, width, height,
                            sBufferInfo.UsrData.sSystemBuffer.iStride[0],
                            sBufferInfo.UsrData.sSystemBuffer.iStride[1]);

    outputPort->onData(rgbBuffer, width * height * bpp, true);

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
