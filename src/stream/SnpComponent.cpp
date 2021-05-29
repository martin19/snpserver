#include "SnpComponent.h"

SnpPort* SnpComponent::getInput(int i) {
    return input.at(i);
}

void SnpComponent::addInput(SnpPort *port) {
    input.push_back(port);
}

SnpPort* SnpComponent::getOutput(int i) {
    return output.at(i);
}

void SnpComponent::addOutput(SnpPort *port) {
    output.push_back(port);
}

SnpComponent::~SnpComponent() {
    for(auto & pPort : input) {
        delete pPort;
    }
    for(auto & pPort : output) {
        delete pPort;
    }
}
