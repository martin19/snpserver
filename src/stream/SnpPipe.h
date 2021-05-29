#ifndef SNPSERVER_SNPPIPE_H
#define SNPSERVER_SNPPIPE_H

#include "SnpComponent.h"

struct SnpPipeOptions {};

class SnpPipe {
public:
    explicit SnpPipe(SnpPipeOptions &options);
    void start();
    void stop();
    bool addComponent(SnpComponent *component);
private:
    std::vector<SnpComponent*> components;
public:
    const std::vector<SnpComponent *> &getComponents() const;

private:
    bool enabled = false;
};


#endif //SNPSERVER_SNPPIPE_H
