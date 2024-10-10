#ifndef SNPSERVER_SNPPIPE_H
#define SNPSERVER_SNPPIPE_H

#include "SnpComponent.h"
#include "SnpProperty.h"
#include "network/snp.pb.h"

struct SnpPipeOptions {
    std::string name;
};

class SnpPipe {
public:
    explicit SnpPipe(SnpPipeOptions &options, uint32_t pipeId);
    bool start();
    void stop();
    bool addComponentBegin(SnpComponent *component);
    bool addComponentEnd(SnpComponent *component);
    const std::vector<SnpComponent *> &getComponents() const;
    uint32_t framesPassed;
    std::vector<SnpProperty*>* getProperties();
protected:
    std::string name;
public:
    const std::string &getName() const;

private:
    std::vector<SnpComponent*> components;
    bool running;
};


#endif //SNPSERVER_SNPPIPE_H
