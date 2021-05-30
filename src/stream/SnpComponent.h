#ifndef SNPSERVER_SNPCOMPONENT_H
#define SNPSERVER_SNPCOMPONENT_H

#include <cstdint>
#include <string>
#include "SnpPort.h"

struct SnpComponentOptions {
};

class SnpComponent {
public:
    explicit SnpComponent(const SnpComponentOptions &options) {};
    virtual ~SnpComponent();
    virtual bool isEnabled() const {
        return enabled;
    }
    virtual void setEnabled(bool enabled) {
        this->enabled = enabled;
    }

    SnpPort *getInputPort(int i);
    SnpPort *getOutputPort(int i);
    void addInputPort(SnpPort *port);
    void addOutputPort(SnpPort *port);

private:
    std::vector<SnpPort*> inputPorts;
    std::vector<SnpPort*> outputPorts;
    bool enabled;
};

#endif //SNPSERVER_SNPCOMPONENT_H
