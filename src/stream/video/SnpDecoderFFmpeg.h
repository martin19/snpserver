#ifndef SNPSERVER_SNPDECODERFFMPEG_H
#define SNPSERVER_SNPDECODERFFMPEG_H

#include <stream/SnpComponent.h>
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

struct SnpDecoderFFmpegOptions : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
};

class SnpDecoderFFmpeg : public SnpComponent {
public:
    explicit SnpDecoderFFmpeg(const SnpDecoderFFmpegOptions &options);
    ~SnpDecoderFFmpeg() override;

    bool start() override;
    void stop() override;

private:
    uint32_t width;
    uint32_t height;
    uint64_t pts;

    void onInputData(uint32_t pipeId, SnpData* data);

    bool ffmpegDecoderInit();
    void ffmpegDecoderDestroy();
    void ffmpegDecoderDecode(const uint8_t *packetData, uint32_t len);

    // FFmpeg objects
    AVCodecContext *dec_ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;
    struct SwsContext *sws_ctx = nullptr;
};

#endif //SNPSERVER_SNPDECODERFFMPEG_H
