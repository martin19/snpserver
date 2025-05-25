#ifndef SNPSERVER_SNPENCODEROPENH264_H
#define SNPSERVER_SNPENCODEROPENH264_H

#include <stream/SnpComponent.h>
#include <string>
#include "codec_api.h"
#include "stream/video/h264/OpenH264Api.h"


struct SnpEncoderOpenH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    double fps;
    uint32_t qp;
};

class SnpEncoderOpenH264 : public SnpComponent {
public:
    explicit SnpEncoderOpenH264(const SnpEncoderOpenH264Options &options);
    ~SnpEncoderOpenH264() override;

    bool start() override;
    void stop() override;

private:
    OpenH264Api openH264Api;

    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    void onInputData(uint32_t pipeId, SnpData* data);

    bool openH264EncoderInit();
    bool openH264EncoderEncode(const uint8_t *framebuffer, uint32_t len);
    void openH264EncoderDestroy();
    ISVCEncoder *encoder;
    uint8_t *yuvBuffer;
};

#endif //SNPSERVER_SNPENCODEROPENH264_H
