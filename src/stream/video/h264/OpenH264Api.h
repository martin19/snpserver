#ifndef SNPSERVER_OPENH264API_H
#define SNPSERVER_OPENH264API_H

#include "codec_api.h"

typedef int (*WelsCreateDecoderFunc)(ISVCDecoder** ppDecoder);
typedef void (*WelsDestroyDecoderFunc)(ISVCDecoder* pDecoder);
typedef int (*WelsCreateSVCEncoderFunc)(ISVCEncoder** ppEncoder);
typedef void (*WelsDestroySVCEncoderFunc)(ISVCEncoder* pEncoder);

struct OpenH264Api {
    WelsCreateDecoderFunc welsCreateDecoderFunc;
    WelsDestroyDecoderFunc  welsDestroyDecoderFunc;
    WelsCreateSVCEncoderFunc welsCreateSVCEncoderFunc;
    WelsDestroySVCEncoderFunc  welsDestroySVCEncoderFunc;
};

#endif //SNPSERVER_OPENH264API_H
