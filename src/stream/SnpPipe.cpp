#include <functional>
#include "SnpPipe.h"
#include "util/loguru.h"

SnpPipe::SnpPipe(SnpPipeOptions &options) {
    componentName = options.name;
    enabled = false;
    running = false;
    framesPassed = 0;
}

void SnpPipe::setEnabled(bool enabled) {
    for(auto & pComponent : components) {
        pComponent->setEnabled(enabled);
    }
    this->enabled = true;
}

bool SnpPipe::start() {
    if(!this->isEnabled()) {
        LOG_F(ERROR, "pipe \"%s\" cannot be started, it must be enabled first.", componentName.c_str());
        return false;
    }
    for(auto & pComponent : components) {
        pComponent->start();
    }
    this->running = true;
    return false;
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

bool SnpPipe::isEnabled() const {
    return enabled;
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
