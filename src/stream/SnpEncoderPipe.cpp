#include <functional>
#include "SnpEncoderPipe.h"
#include "SnpSinkNetwork.h"
#include "SnpSourceModesetting.h"
#include "SnpEncoderMmalH264.h"

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

void SnpEncoderPipe::start() {
    this->running = true;
    this->encoder->setOnFrameDataCb(std::bind(&SnpEncoderPipe::onFrameDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    while(this->running) {
        if(this->source->isFrameReady()) {
            this->encoder->process();
        }
    }
}

void SnpEncoderPipe::onFrameDataCb(uint8_t *buffer, int len, bool complete) {
    this->sink->process(buffer, len, complete);
}

void SnpEncoderPipe::stop() {
    this->running = false;
}
