#ifndef SNPSERVER_SNPENCODERAMFH264_H
#define SNPSERVER_SNPENCODERAMFH264_H

#include "stream/SnpComponent.h"
#include "public/include/core/Context.h"
#include "public/include/components/Component.h"

struct SnpEncoderAmfH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    double fps;
    uint32_t qp;
};

class SnpEncoderAmfH264 : public SnpComponent {
public:
    SnpEncoderAmfH264(const SnpEncoderAmfH264Options &options);
    ~SnpEncoderAmfH264() override;

    bool start() override;
    void stop() override;

private:
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    void onInputData(uint32_t pipeId, SnpData *data);

    amf::AMFContextPtr context;
    amf::AMFComponentPtr encoder;
    amf::AMFSurfacePtr surfaceIn;

//    bool openH264EncoderInit();
//    bool openH264EncoderEncode(const uint8_t *framebuffer, uint32_t len);
//    void openH264EncoderDestroy();
//    ISVCEncoder *encoder;
//    uint8_t *yuvBuffer;
    void fillRGBASurface(amf::AMFSurface *pSurface, uint8_t *rgba);
};


#endif //SNPSERVER_SNPENCODERAMFH264_H
