#include "SnpSinkDisplay.h"

SnpSinkDisplay::SnpSinkDisplay(const SnpSinkDisplayOptions &options) : SnpComponent(options, "sinkDisplay") {
    streamId = options.streamId;
    width = options.width;
    height = options.height;

    addInputPort(new SnpPort());
}

SnpSinkDisplay::~SnpSinkDisplay() {
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