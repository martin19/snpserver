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

bool SnpComponent::start() {
    LOG_F(INFO, "starting component \"%s\"", name.c_str());
    running = true;
    return true;
}

void SnpComponent::stop() {
    LOG_F(INFO, "stopping component \"%s\"", name.c_str());
    running = false;
}

const std::string &SnpComponent::getName() const {
    return name;
}

uint32_t SnpComponent::getPipeId() const {
    return pipeId;
}

void SnpComponent::setPipeId(uint32_t pipeId) {
    SnpComponent::pipeId = pipeId;
}

const std::vector<SnpPort *> &SnpComponent::getInputPorts() const {
    return inputPorts;
}

const std::vector<SnpPort *> &SnpComponent::getOutputPorts() const {
    return outputPorts;
}
