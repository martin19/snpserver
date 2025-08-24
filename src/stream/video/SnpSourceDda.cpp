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

    const AVInputFormat *fmt = nullptr;
    while ((fmt = av_input_audio_device_next(fmt))) {
        printf("%s\n", fmt->name);
        if (!strcmp(fmt->name, "lavfi"))
            break;
    }

    double fps = getProperty("fps")->getValueDouble();

    AVDictionary *optionsDict = nullptr;

    std::string filtergraph = "ddagrab=output_idx=1:framerate=60,hwdownload,format=bgra";

    int ret = avformat_open_input(&fmt_ctx, filtergraph.c_str(), fmt, nullptr);
    if (ret < 0) {
        char err[256];
        av_strerror(ret, err, sizeof(err));
        LOG_F(ERROR, "Failed to open lavfi/ddagrab: %s", err);
        return false;
    }

    // Retrieve stream info
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        LOG_F(ERROR, "Failed to find stream info.");
        return false;
    }

    // Find video stream
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index < 0) {
        LOG_F(ERROR, "No video stream found!");
        return false;
    }

    // Find decoder
    AVCodec *decoder = const_cast<AVCodec *>(avcodec_find_decoder(
            fmt_ctx->streams[video_stream_index]->codecpar->codec_id));
    if (!decoder) {
        LOG_F(ERROR, "No suitable decoder found.");
        return false;
    }

    codec_ctx = avcodec_alloc_context3(decoder);
    if (!codec_ctx) return false;

    avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);
    if (avcodec_open2(codec_ctx, decoder, nullptr) < 0) {
        LOG_F(ERROR, "Failed to open decoder.");
        return false;
    }

    running = true;
    worker = std::thread(&SnpSourceDda::captureLoop, this);

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

void SnpSourceDda::captureLoop() {
    AVFrame *frame = av_frame_alloc();
    while (running) {
        if (grabFrame(frame)) {
            // Push frame to output port or process it
            SnpDataRam data(frame->data[0], codec_ctx->width * codec_ctx->height * 4, true);
            getOutputPort(0)->onData(getPipeId(), &data);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    av_frame_free(&frame);
}

bool SnpSourceDda::grabFrame(AVFrame *frame) {
    AVPacket pkt;
    av_init_packet(&pkt);

    if (av_read_frame(fmt_ctx, &pkt) < 0)
        return false;

    if (pkt.stream_index == video_stream_index) {
        int ret = avcodec_send_packet(codec_ctx, &pkt);
        if (ret >= 0)
            ret = avcodec_receive_frame(codec_ctx, frame);
        av_packet_unref(&pkt);
        return ret >= 0;
    }

    av_packet_unref(&pkt);
    return false;
}

