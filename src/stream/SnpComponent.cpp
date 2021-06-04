#include "SnpComponent.h"

SnpPort* SnpComponent::getInputPort(int i) {
    return inputPorts.at(i);
}

void SnpComponent::addInputPort(SnpPort *port) {
    port->setOwner(this);
    inputPorts.push_back(port);
}

SnpPort* SnpComponent::getOutputPort(int i) {
    return outputPorts.at(i);
}

void SnpComponent::addOutputPort(SnpPort *port) {
    port->setOwner(this);
    outputPorts.push_back(port);
}

SnpComponent::~SnpComponent() {
    for(auto & pPort : inputPorts) {
        delete pPort;
    }
    for(auto & pPort : outputPorts) {
        delete pPort;
    }
}

uint32_t SnpComponent::getTimestampStartMs() const {
    return timestampStartMs;
}

void SnpComponent::setTimestampStartMs(uint32_t timestampStartMs) {
    SnpComponent::timestampStartMs = timestampStartMs;
}

uint32_t SnpComponent::getTimestampEndMs() const {
    return timestampEndMs;
}

void SnpComponent::setTimestampEndMs(uint32_t timestampEndMs) {
    SnpComponent::timestampEndMs = timestampEndMs;
}

SnpPipe *SnpComponent::getOwner() const {
    return owner;
}

void SnpComponent::setOwner(SnpPipe *owner) {
    SnpComponent::owner = owner;
}
