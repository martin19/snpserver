#include "SnpDecoderFFmpeg.h"
#include "stream/data/SnpDataRam.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

SnpDecoderFFmpeg::SnpDecoderFFmpeg(const SnpDecoderFFmpegOptions &options) : SnpComponent(options, "COMPONENT_DECODER_FFMPEG") {
    addInputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_H264));
    addOutputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
//    addProperty(new SnpProperty("qp", options.qp));
    getInputPort(0)->setOnDataCb(std::bind(&SnpDecoderFFmpeg::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2));
}

SnpDecoderFFmpeg::~SnpDecoderFFmpeg() {
    ffmpegDecoderDestroy();
}

bool SnpDecoderFFmpeg::start() {
    SnpComponent::start();
    return ffmpegDecoderInit();
}

void SnpDecoderFFmpeg::stop() {
    SnpComponent::stop();
    ffmpegDecoderDestroy();
}

void SnpDecoderFFmpeg::onInputData(uint32_t pipeId, SnpData* data) {
    if (!isRunning()) return;
    if (auto* ram = dynamic_cast<SnpDataRam*>(data)) {
        ffmpegDecoderDecode(ram->getData(), ram->getLen());
    }
}

bool SnpDecoderFFmpeg::ffmpegDecoderInit() {
    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        LOG_F(ERROR, "H264 decoder not found");
        return false;
    }

    dec_ctx = avcodec_alloc_context3(codec);
    if (!dec_ctx) return false;

    if (avcodec_open2(dec_ctx, codec, nullptr) < 0) {
        avcodec_free_context(&dec_ctx);
        return false;
    }

    frame = av_frame_alloc();
    pkt = av_packet_alloc();

    return frame && pkt;
}

void SnpDecoderFFmpeg::ffmpegDecoderDestroy() {
    if (sws_ctx) sws_freeContext(sws_ctx);
    if (pkt) av_packet_free(&pkt);
    if (frame) av_frame_free(&frame);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
}

void SnpDecoderFFmpeg::ffmpegDecoderDecode(const uint8_t *packetData, uint32_t len) {


    av_packet_unref(pkt);
    av_new_packet(pkt, (int)len);
    memcpy(pkt->data, packetData, len);

    if (avcodec_send_packet(dec_ctx, pkt) < 0) {
        LOG_F(ERROR, "Error sending packet to decoder");
        return;
    }

    while (avcodec_receive_frame(dec_ctx, frame) == 0) {
        // convert YUV420P → RGBA
        uint8_t *dst_data[4];
        int dst_linesize[4];
        av_image_alloc(dst_data, dst_linesize, (int)width, (int)height, AV_PIX_FMT_RGBA, 1);

        if (!sws_ctx) {
            sws_ctx = sws_getContext(
                    (int)width, (int)height, (AVPixelFormat)frame->format,
                    (int)width, (int)height, AV_PIX_FMT_RGBA,
                    SWS_BILINEAR, nullptr, nullptr, nullptr
            );
        }

        sws_scale(sws_ctx, frame->data, frame->linesize, 0, (int)height, dst_data, dst_linesize);

        SnpDataRam out(dst_data[0], width * height * 4, true);
        getOutputPort(0)->onData(getPipeId(), &out);

        av_freep(&dst_data[0]);
    }
}

