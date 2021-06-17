#include "SnpComponent.h"
#include "util/loguru.h"

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
    for(auto &property : properties) {
        delete property.second;
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

void SnpComponent::addProperty(SnpProperty *property) {
    properties.insert(std::pair<std::string, SnpProperty*>(property->getName(), property));
}

SnpProperty *SnpComponent::getProperty(std::string name) {
    auto entry = properties.find(name);
    SnpProperty *property = entry->second;
    return property;
}

std::map<std::string, SnpProperty *> &SnpComponent::getProperties() {
    return properties;
}

bool SnpComponent::isRunning() const {
    return running;
}

bool SnpComponent::isEnabled() const {
    return enabled;
}

void SnpComponent::setEnabled(bool enabled)  {
    this->enabled = enabled;
}

bool SnpComponent::start() {
    if(this->isEnabled()) {
        LOG_F(ERROR, "Component \"%s\" cannot be started because it has not been enabled.", componentName.c_str());
        return false;
    } else {
        running = true;
    }
    return true;
}
