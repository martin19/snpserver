#include <iostream>
#include <util/VideoUtil.h>
#include "util/assert.h"
#include "stream/data/SnpDataRam.h"
#include "SnpEncoderFFmpeg.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


SnpEncoderFFmpeg::SnpEncoderFFmpeg(const SnpEncoderFFmpegOptions &options) : SnpComponent(options, "COMPONENT_ENCODER_FFMPEG") {

    addInputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    addOutputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_H264));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("qp", options.qp));

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderFFmpeg::onInputData, this, std::placeholders::_1, std::placeholders::_2));
}

SnpEncoderFFmpeg::~SnpEncoderFFmpeg() {
    ffmpegEncoderDestroy();
}

bool SnpEncoderFFmpeg::start() {
    SnpComponent::start();

    ffmpegEncoderInit();
    return true;
}

void SnpEncoderFFmpeg::stop() {
    SnpComponent::stop();
    ffmpegEncoderDestroy();
}

void SnpEncoderFFmpeg::onInputData(uint32_t pipeId, SnpData* data) {
    if(!isRunning()) return;
    if(auto* ram = dynamic_cast<SnpDataRam*>(data)) {
        this->ffmpegEncoderEncode(ram->getData(), ram->getLen());
    }
}

bool SnpEncoderFFmpeg::ffmpegEncoderInit() {
    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();
    double fps = getProperty("fps")->getValueDouble();

    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        LOG_F(ERROR, "codec AV_CODEC_ID_H264 not found");
        return false;
    }

    enc_ctx = avcodec_alloc_context3(codec);
    if (!enc_ctx) {
        LOG_F(ERROR, "failed to alloc context");
        return false;
    }

    enc_ctx->width = width;
    enc_ctx->height = height;
    enc_ctx->time_base = AVRational{1, static_cast<int>(fps)};
    enc_ctx->framerate = AVRational{static_cast<int>(fps), 1};
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // quality / bitrate settings
    enc_ctx->gop_size = 12;
    enc_ctx->max_b_frames = 2;
    av_opt_set(enc_ctx->priv_data, "preset", "veryfast", 0);

    if (avcodec_open2(enc_ctx, codec, nullptr) < 0) {
        // log error
        avcodec_free_context(&enc_ctx);
        return false;
    }

    // allocate frame
    frame = av_frame_alloc();
    if (!frame) {
        LOG_F(ERROR, "failed to allocate frame");
        return false;
    }
    frame->format = enc_ctx->pix_fmt;
    frame->width  = enc_ctx->width;
    frame->height = enc_ctx->height;

    if (av_frame_get_buffer(frame, 32) < 0) {
        LOG_F(ERROR, "failed to allocate frame buffer");
        return false;
    }

    // allocate packet
    pkt = av_packet_alloc();
    if (!pkt) {
        LOG_F(ERROR, "failed to allocate packet");
        return false;
    }

    pts = 0;

    return true;
}


void SnpEncoderFFmpeg::ffmpegEncoderDestroy() {
    if (pkt) {
        av_packet_free(&pkt);
    }
    if (frame) {
        av_frame_free(&frame);
    }
    if (enc_ctx) {
        avcodec_free_context(&enc_ctx);
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
}

void SnpEncoderFFmpeg::ffmpegEncoderEncode(const uint8_t *framebuffer, uint32_t len) {
    uint8_t *src_rgba = const_cast<uint8_t *>(framebuffer);
    int src_linesize[1] = { 4 * (int)width }; // RGBA stride

    if (!sws_ctx) {
        sws_ctx = sws_getContext(
                width, height, AV_PIX_FMT_RGBA,
                width, height, AV_PIX_FMT_YUV420P,
                SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    if (av_frame_make_writable(frame) < 0) {
        LOG_F(ERROR, "frame not writable");
        return;
    }

    // convert RGBA → YUV420P
    sws_scale(sws_ctx,
              &src_rgba,
              src_linesize,
              0, height,
              frame->data,
              frame->linesize);

    frame->pts = pts++;

    // send frame to encoder
    if (avcodec_send_frame(enc_ctx, frame) < 0) {
        LOG_F(ERROR, "error sending frame to encoder");
        return;
    }

    // read all available packets
    while (avcodec_receive_packet(enc_ctx, pkt) == 0) {
        // push encoded packet downstream
        SnpDataRam ram(pkt->data, pkt->size, true);
        getOutputPort(0)->onData(getPipeId(), &ram);

        av_packet_unref(pkt);
    }
}