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