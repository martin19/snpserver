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
    explicit SnpComponent(const SnpComponentOptions &options, const std::string& name) {
        LOG_F(INFO, "Initializing component %s", name.c_str());
        this->name = name;
        pipeId = options.pipeId;
        running = false;
    };
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
protected:
    std::string name;
public:
    const std::string &getName() const;
private:
    uint32_t pipeId;
public:
    uint32_t getPipeId() const;

    void setPipeId(uint32_t pipeId);

private:
    bool running;
    std::vector<SnpPort*> inputPorts;
    std::vector<SnpPort*> outputPorts;
public:
    const std::vector<SnpPort *> &getInputPorts() const;

    const std::vector<SnpPort *> &getOutputPorts() const;

private:

    uint32_t timestampStartMs;
    uint32_t timestampEndMs;
    SnpPipe *owner;

    std::map<std::string, SnpProperty*> properties;
};

#endif //SNPSERVER_SNPCOMPONENT_H
