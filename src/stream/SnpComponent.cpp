#include "SnpComponent.h"

SnpPort* SnpComponent::getInputPort(int i) {
    return inputPorts.at(i);
}

void SnpComponent::addInputPort(SnpPort *port) {
    inputPorts.push_back(port);
}

SnpPort* SnpComponent::getOutputPort(int i) {
    return outputPorts.at(i);
}

void SnpComponent::addOutputPort(SnpPort *port) {
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
