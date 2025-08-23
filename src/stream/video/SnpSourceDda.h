#ifndef SNPSERVER_SNPSOURCEDDA_H
#define SNPSERVER_SNPSOURCEDDA_H

#include "stream/SnpComponent.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
}

struct SnpSourceDdaOptions : public SnpComponentOptions {
    double fps;
    uint32_t width;
    uint32_t height;
};

class SnpSourceDda : public SnpComponent {
public:
    explicit SnpSourceDda(const SnpSourceDdaOptions &options);
    ~SnpSourceDda() override;
    bool start() override;
    void stop() override;
private:
    bool grabFrame(AVFrame *frame);
    AVFormatContext *fmt_ctx;
    AVCodecContext *codec_ctx;
    int video_stream_index;

    std::thread worker;   // background capture thread
    std::atomic<bool> running {false};  // thread-safe flag
};

#endif //SNPSERVER_SNPSOURCEDDA_H
