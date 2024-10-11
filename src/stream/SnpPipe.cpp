#include <functional>
#include "SnpPipe.h"
#include "util/loguru.h"

SnpPipe::SnpPipe(SnpPipeOptions &options, uint32_t pipeId) {
    this->pipeId = pipeId;
    name = options.name;
    running = false;
    framesPassed = 0;
}

bool SnpPipe::start() {
    LOG_F(INFO, "starting pipe %d", pipeId);
    for(int i = (int)components.size()-1; i >= 0; i--) {
        if(!components[i]->isRunning()) components[i]->start();
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

bool SnpPipe::addComponentBegin(SnpComponent *component) {
    component->setOwner(this);
    components.insert(components.begin(), component);
    if(components.size() > 1) {
        SnpComponent* nextComponent = components.at(1);
        if(SnpPort::canConnect(nextComponent->getInputPort(0), component->getOutputPort(0))) {
            SnpPort::connect(pipeId, component->getOutputPort(0), nextComponent->getInputPort(0));
            return true;
        } else {
            LOG_F(ERROR, "cannot connect incompatible components %s -> %s",
                  component->getName().c_str(), nextComponent->getName().c_str() );
            return false;
        }
    }
    return true;
}

bool SnpPipe::addComponentEnd(SnpComponent *component) {
    component->setOwner(this);
    components.push_back(component);
    //connect to previous component if one exists
    if(components.size() > 1) {
        SnpComponent* previousComponent = components.at(components.size()-2);
        if(SnpPort::canConnect(previousComponent->getOutputPort(0), component->getInputPort(0))) {
            SnpPort::connect(pipeId, previousComponent->getOutputPort(0), component->getInputPort(0));
            return true;
        } else {
            LOG_F(ERROR, "cannot connect incompatible components %s -> %s",
                  previousComponent->getName().c_str(), component->getName().c_str() );
            return false;
        }
    }
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
