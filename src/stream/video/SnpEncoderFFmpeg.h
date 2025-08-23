#ifndef SNPSERVER_SNPENCODERFFMPEG_H
#define SNPSERVER_SNPENCODERFFMPEG_H

#include <stream/SnpComponent.h>
#include <string>

struct SnpEncoderFFmpegOptions : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    double fps;
    uint32_t qp;
};

class SnpEncoderFFmpeg : public SnpComponent {
public:
    explicit SnpEncoderFFmpeg(const SnpEncoderFFmpegOptions &options);
    ~SnpEncoderFFmpeg() override;

    bool start() override;
    void stop() override;

private:
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    void onInputData(uint32_t pipeId, SnpData* data);

//    bool openH264EncoderInit();
//    bool openH264EncoderEncode(const uint8_t *framebuffer, uint32_t len);
//    void openH264EncoderDestroy();
//    uint8_t *yuvBuffer;
};

#endif //SNPSERVER_SNPENCODERFFMPEG_H
