#ifndef SNPSERVER_SNPENCODERFFMPEG_H
#define SNPSERVER_SNPENCODERFFMPEG_H

#include <stream/SnpComponent.h>
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

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

    AVCodecContext *enc_ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    SwsContext *sws_ctx = nullptr;
    int64_t pts = 0;

    void onInputData(uint32_t pipeId, SnpData* data);
    bool ffmpegEncoderInit();
    void ffmpegEncoderDestroy();
    void ffmpegEncoderEncode(const uint8_t *framebuffer, uint32_t len);
};

#endif //SNPSERVER_SNPENCODERFFMPEG_H
