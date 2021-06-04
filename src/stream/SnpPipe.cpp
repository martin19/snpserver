#include <functional>
#include "SnpPipe.h"

SnpPipe::SnpPipe(SnpPipeOptions &options) {
    framesPassed = 0;
}

void SnpPipe::start() {
    for(auto & pComponent : components) {
        pComponent->setEnabled(true);
    }
    this->enabled = true;
}

void SnpPipe::stop() {
    for(auto & pComponent : components) {
        pComponent->setEnabled(false);
    }
    this->enabled = false;
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

void SnpPipe::setEnabled(bool enabled) {
    SnpPipe::enabled = enabled;
}
