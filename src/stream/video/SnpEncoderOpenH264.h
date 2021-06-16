#ifndef SNPSERVER_SNPENCODEROPENH264_H
#define SNPSERVER_SNPENCODEROPENH264_H

#include <stream/SnpComponent.h>
#include <string>
#include "codec_api.h"

struct SnpEncoderOpenH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerPixel;
    uint32_t qp;
};

class SnpEncoderOpenH264 : public SnpComponent {
public:
    explicit SnpEncoderOpenH264(const SnpEncoderOpenH264Options &options);
    ~SnpEncoderOpenH264() override;

    void setEnabled(bool enabled) override;

private:
    uint32_t width;
    uint32_t height;
    uint32_t bpp;


    void onInputData(const uint8_t *data, uint32_t len, bool complete);

    bool openH264EncoderInit();
    bool openH264EncoderEncode(const uint8_t *framebuffer, uint32_t len);
    void openH264EncoderDestroy();
    ISVCEncoder *encoder;
    uint8_t *yuvBuffer;
};

#endif //SNPSERVER_SNPENCODEROPENH264_H
