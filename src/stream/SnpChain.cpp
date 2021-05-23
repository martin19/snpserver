#include <functional>
#include "SnpPipeline.h"
#include "SnpSinkNetwork.h"
#include "SnpSourceModesetting.h"
#include "SnpEncoderMmalH264.h"

SnpPipeline::SnpPipeline(SnpPipelineOptions &options) {
    this->source = options.source;
    this->encoder = options.encoder;
    this->sink = options.sink;
}

SnpSource *SnpPipeline::getSource() {
    return this->source;
}

SnpEncoder *SnpPipeline::getEncoder() {
    return this->encoder;
}

SnpSink *SnpPipeline::getSink() {
    return this->sink;
}

void SnpPipeline::start() {
    this->running = true;
    this->encoder->setOnFrameDataCb(std::bind(&SnpPipeline::onFrameDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    while(this->running) {
        if(this->source->isFrameReady()) {
            this->encoder->process();
        }
    }
}

void SnpPipeline::onFrameDataCb(uint8_t *buffer, int len, bool complete) {
    this->sink->process(buffer, len, complete);
}

void SnpPipeline::stop() {
    this->running = false;
}
