#include <functional>
#include "SnpPipe.h"

SnpPipe::SnpPipe(SnpPipeOptions &options) {
}

//void SnpPipe::onSourceFrameDataCb(uint8_t *buffer, int len, bool complete) {
//    this->encoder->process(buffer, len, complete);
//}
//
//void SnpPipe::onEncoderFrameDataCb(uint8_t *buffer, int len, bool complete) {
//    this->sink->process(buffer, len, complete);
//}

void SnpPipe::start() {
//    this->source->setOnFrameDataCb(std::bind(&SnpPipe::onSourceFrameDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//    this->encoder->setOnFrameDataCb(std::bind(&SnpPipe::onEncoderFrameDataCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//    this->sink->setEnabled(true);
//    this->encoder->setEnabled(true);
//    this->source->setEnabled(true);
//    this->enabled = true;
}

void SnpPipe::stop() {
//    this->enabled = false;
//    this->source->setEnabled(false);
//    this->encoder->setEnabled(false);
//    this->sink->setEnabled(false);
//    this->encoder->setOnFrameDataCb(nullptr);
//    this->source->setOnFrameDataCb(nullptr);
}

bool SnpPipe::addComponent(SnpComponent *component) {
    components.push_back(component);
    return true;
}

const std::vector<SnpComponent *> &SnpPipe::getComponents() const {
    return components;
}
