#ifndef SNPSERVER_SNPCOMPONENT_H
#define SNPSERVER_SNPCOMPONENT_H

#include <cstdint>
#include <string>
#include "SnpPort.h"
#include "SnpPipe.h"

class SnpPipe;

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

    uint32_t getTimestampStartMs() const;
    void setTimestampStartMs(uint32_t timestampStartMs);
    uint32_t getTimestampEndMs() const;
    void setTimestampEndMs(uint32_t timestampEndMs);
    SnpPipe *getOwner() const;
    void setOwner(SnpPipe *owner);

private:
    std::vector<SnpPort*> inputPorts;
    std::vector<SnpPort*> outputPorts;
    bool enabled;
    uint32_t timestampStartMs;
    uint32_t timestampEndMs;
    SnpPipe *owner;
};

#endif //SNPSERVER_SNPCOMPONENT_H
