#include "SnpSinkDisplay.h"

SnpSinkDisplay::SnpSinkDisplay(const SnpSinkDisplayOptions &options) : SnpComponent(options, "sinkDisplay") {
    streamId = options.streamId;
    width = options.width;
    height = options.height;

    qImage = new QImage((int)width, (int)height, QImage::Format_RGB888);

    addInputPort(new SnpPort());
    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkDisplay::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpSinkDisplay::~SnpSinkDisplay() {
    delete qImage;
}

bool SnpSinkDisplay::start() {
    SnpComponent::start();
    return true;
}

void SnpSinkDisplay::stop() {
    SnpComponent::stop();
}

void SnpSinkDisplay::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
}

void SnpSinkDisplay::onInputData(const uint8_t *data, int len, bool complete) {
    memcpy(qImage->bits(), data, len);
}

QImage *SnpSinkDisplay::getQImage() const {
    return qImage;
}
