#include <functional>
#include "SnpEncoderPipe.h"
#include "network/SnpSinkNetwork.h"
#include "video/SnpSourceModesetting.h"
#include "video/SnpEncoderMmalH264.h"

SnpEncoderPipe::SnpEncoderPipe(SnpPipelineOptions &options) {
    this->source = options.source;
    this->encoder = options.encoder;
    this->sink = options.sink;
}

SnpSource *SnpEncoderPipe::getSource() {
    return this->source;
}

SnpEncoder *SnpEncoderPipe::getEncoder() {
    return this->encoder;
}

SnpSink *SnpEncoderPipe::getSink() {
    return this->sink;
}

void SnpEncoderPipe::onSourceFrameDataCb(uint8_t *buffer, int len, bool complete) {
    this->encoder->process(buffer, len, complete);
}

void SnpEncoderPipe::onEncoderFrameDataCb(uint8_t *buffer, int len, bool complete) {
    this->sink->process(buffer, len, complete);
}

void SnpEncoderPipe::start() {
    this->source->setOnFrameDataCb(std::bind(&SnpEncoderPipe::onSourceFrameDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->encoder->setOnFrameDataCb(std::bind(&SnpEncoderPipe::onEncoderFrameDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->sink->setEnabled(true);
    this->encoder->setEnabled(true);
    this->source->setEnabled(true);
    this->enabled = true;
}

void SnpEncoderPipe::stop() {
    this->enabled = false;
    this->source->setEnabled(false);
    this->encoder->setEnabled(false);
    this->sink->setEnabled(false);
    this->encoder->setOnFrameDataCb(nullptr);
    this->source->setOnFrameDataCb(nullptr);
}