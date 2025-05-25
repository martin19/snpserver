#include "SnpSinkDisplay.h"

SnpSinkDisplay::SnpSinkDisplay(const SnpSinkDisplayOptions &options) : SnpComponent(options, "COMPONENT_OUTPUT_VIDEO_DISPLAY") {
    addInputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    width = options.width;
    height = options.height;
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