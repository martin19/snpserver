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
    bool isEnabled() const {
        return enabled;
    }
    void setEnabled(bool enabled) {
        this->enabled = enabled;
    }

    SnpPort *getInput(int i);
    SnpPort *getOutput(int i);
    void addInput(SnpPort *port);
    void addOutput(SnpPort *port);

private:
    std::vector<SnpPort*> input;
    std::vector<SnpPort*> output;
    bool enabled;
};

#endif //SNPSERVER_SNPCOMPONENT_H
