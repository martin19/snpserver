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
    explicit SnpPipe(SnpPipeOptions &options);
    bool start();
    void stop();
    bool addComponent(SnpComponent *component);
    const std::vector<SnpComponent *> &getComponents() const;
    uint32_t framesPassed;
    std::vector<SnpProperty*>* getProperties();
protected:
    std::string componentName;
private:
    std::vector<SnpComponent*> components;
    bool running;
};


#endif //SNPSERVER_SNPPIPE_H
