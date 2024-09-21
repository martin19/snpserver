#ifndef SNPSERVER_SNPDECODEROPENH264_H
#define SNPSERVER_SNPDECODEROPENH264_H

#include <stream/SnpComponent.h>
#include <string>
#include "codec_api.h"
#include "stream/video/h264/OpenH264Api.h"

struct SnpDecoderOpenH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerPixel;
    uint32_t qp;
};

class SnpDecoderOpenH264 : public SnpComponent {
public:
    explicit SnpDecoderOpenH264(const SnpDecoderOpenH264Options &options);
    ~SnpDecoderOpenH264() override;

    void setEnabled(bool enabled) override;

private:
    OpenH264Api openH264Api;

    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    void onInputData(const uint8_t *data, uint32_t len, bool complete);

    bool openH264DecoderInit();
    bool openH264DecoderDecode(const uint8_t *srcBuffer, int srcLen);
    void openH264DecoderDestroy();
    ISVCDecoder *decoder;
    uint8_t *yuvBuffer[3];
    uint8_t *rgbBuffer;
};


#endif //SNPSERVER_SNPDECODEROPENH264_H
