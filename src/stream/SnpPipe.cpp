#include <functional>
#include "SnpPipe.h"
#include "util/loguru.h"

SnpPipe::SnpPipe(SnpPipeOptions &options) {
    componentName = options.name;
    running = false;
    framesPassed = 0;
}

bool SnpPipe::start() {
    LOG_F(INFO, "starting pipe \"%s\"", componentName.c_str());
    for(auto & pComponent : components) {
        pComponent->start();
    }
    this->running = true;
    return true;
}

void SnpPipe::stop() {
    for(auto & pComponent : components) {
        pComponent->stop();
    }
    this->running = true;
}

bool SnpPipe::addComponent(SnpComponent *component) {
    component->setOwner(this);
    components.push_back(component);
    return true;
}

const std::vector<SnpComponent *> &SnpPipe::getComponents() const {
    return components;
}

std::vector<SnpProperty*>* SnpPipe::getProperties() {
    auto* properties = new std::vector<SnpProperty*>();
    for(auto & pComponent : components) {
        for(auto & pProperty : pComponent->getProperties()) {
            properties->push_back(pProperty.second);
        }
    }
    return properties;
}

snappyv1::StreamMedium SnpPipe::getMedium() const {
    return medium;
}

void SnpPipe::setMedium(snappyv1::StreamMedium medium) {
    SnpPipe::medium = medium;
}

snappyv1::StreamEndpoint SnpPipe::getEndpoint() const {
    return endpoint;
}

void SnpPipe::setEndpoint(snappyv1::StreamEndpoint endpoint) {
    SnpPipe::endpoint = endpoint;
}

snappyv1::StreamEncoding SnpPipe::getEncoding() const {
    return encoding;
}

void SnpPipe::setEncoding(snappyv1::StreamEncoding encoding) {
    SnpPipe::encoding = encoding;
}

snappyv1::StreamDirection SnpPipe::getDirection() const {
    return direction;
}

void SnpPipe::setDirection(snappyv1::StreamDirection direction) {
    SnpPipe::direction = direction;
}
