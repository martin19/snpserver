#include <thread>
#include "SnpSourceDda.h"
#include "stream/data/SnpDataRam.h"

SnpSourceDda::SnpSourceDda(const SnpSourceDdaOptions &options) : SnpComponent(options, "COMPONENT_CAPTURE_VIDEO_DDA") {
    addOutputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));

    fmt_ctx = nullptr;
    codec_ctx = nullptr;
    video_stream_index = -1;
}

SnpSourceDda::~SnpSourceDda() {
    stop();
}

bool SnpSourceDda::start() {
    AVInputFormat *input_fmt = const_cast<AVInputFormat *>(av_find_input_format("d3d11dup"));
    if (!input_fmt) {
        LOG_F(ERROR, "FFmpeg build does not support d3d11dup input format");
        return false;
    }

    // "desktop" means the primary monitor. You can also specify \\.\DISPLAY1 etc.
    if (avformat_open_input(&fmt_ctx, "desktop", input_fmt, nullptr) < 0) {
        LOG_F(ERROR, "Could not open desktop capture (d3d11dup)");
        return false;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        LOG_F(ERROR, "Could not find stream info");
        return false;
    }

    // Find video stream
    for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        LOG_F(ERROR, "No video stream found in desktop capture");
        return false;
    }

    // Setup codec context
    AVCodecParameters *codecpar = fmt_ctx->streams[video_stream_index]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        LOG_F(ERROR, "Could not open codec for desktop capture");
        return false;
    }

    running = true;

    // start a thread that continuously grabs frames
    worker = std::thread([this]() {
        AVFrame *frame = av_frame_alloc();
        while (running) {
            if (grabFrame(frame)) {
                SnpDataRam data(frame->data[0], codec_ctx->width * codec_ctx->height * 4, true);
                getOutputPort(0)->onData(getPipeId(), &data);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        av_frame_free(&frame);
    });

    return true;
}

void SnpSourceDda::stop() {
    running = false;
    if (worker.joinable())
        worker.join();

    if (codec_ctx) {
        avcodec_free_context(&codec_ctx);
        codec_ctx = nullptr;
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = nullptr;
    }
}

bool SnpSourceDda::grabFrame(AVFrame *frame) {
    AVPacket pkt;
    av_init_packet(&pkt);

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_index) {
            int ret = avcodec_send_packet(codec_ctx, &pkt);
            if (ret < 0) {
                av_packet_unref(&pkt);
                return false;
            }

            ret = avcodec_receive_frame(codec_ctx, frame);
            av_packet_unref(&pkt);

            if (ret == 0) {
                return true;
            }
        }
        av_packet_unref(&pkt);
    }
    return false;
}

