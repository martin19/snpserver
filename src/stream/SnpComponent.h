#ifndef SNPSERVER_SNPCOMPONENT_H
#define SNPSERVER_SNPCOMPONENT_H

#include <cstdint>
#include <string>
#include <map>
#include "SnpPort.h"
#include "SnpPipe.h"
#include "SnpProperty.h"

class SnpPipe;

struct SnpComponentOptions {
    std::string name;
};

class SnpComponent {
public:
    explicit SnpComponent(const SnpComponentOptions &options) {
        this->componentName = options.name;
        enabled = false;
        running = false;
    };
    virtual ~SnpComponent();

    virtual void setEnabled(bool enabled);
    bool isEnabled() const;
    bool isRunning() const;
    virtual bool start();
    virtual void stop() {
        running = false;
    }

    SnpPort *getInputPort(int i);
    SnpPort *getOutputPort(int i);
    void addInputPort(SnpPort *port);
    void addOutputPort(SnpPort *port);

    void addProperty(SnpProperty *property);
    SnpProperty *getProperty(std::string name);
    std::map<std::string, SnpProperty *> &getProperties();

    uint32_t getTimestampStartMs() const;
    void setTimestampStartMs(uint32_t timestampStartMs);
    uint32_t getTimestampEndMs() const;
    void setTimestampEndMs(uint32_t timestampEndMs);
    SnpPipe *getOwner() const;
    void setOwner(SnpPipe *owner);
protected:
    std::string componentName;
private:
    bool running;
    bool enabled;
    std::vector<SnpPort*> inputPorts;
    std::vector<SnpPort*> outputPorts;

    uint32_t timestampStartMs;
    uint32_t timestampEndMs;
    SnpPipe *owner;

    std::map<std::string, SnpProperty*> properties;
};

#endif //SNPSERVER_SNPCOMPONENT_H
