#ifndef SNPSERVER_SNPCOMPONENT_H
#define SNPSERVER_SNPCOMPONENT_H

#include <cstdint>
#include <string>
#include <map>
#include "SnpPort.h"
#include "SnpProperty.h"
#include "util/loguru.h"

class SnpPipe;

struct SnpComponentOptions {
    uint32_t pipeId;
};

class SnpComponent {
public:
    explicit SnpComponent(const SnpComponentOptions &options, const std::string& name);;
    virtual ~SnpComponent();

    bool isRunning() const;
    virtual bool start();
    virtual void stop();

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
    const std::vector<SnpPort *> &getInputPorts() const;
    const std::vector<SnpPort *> &getOutputPorts() const;
    const std::string &getName() const;
    uint32_t getPipeId() const;
    void setPipeId(uint32_t pipeId);
protected:
    std::string name;
private:
    uint32_t pipeId;
    bool running;
    std::vector<SnpPort*> inputPorts;
    std::vector<SnpPort*> outputPorts;
    uint32_t timestampStartMs;
    uint32_t timestampEndMs;
    SnpPipe *owner;

    std::map<std::string, SnpProperty*> properties;
};

#endif //SNPSERVER_SNPCOMPONENT_H
