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
    uint32_t framesPassed;
private:
    bool enabled = false;
public:
    bool isEnabled() const;

    void setEnabled(bool enabled);
};


#endif //SNPSERVER_SNPPIPE_H
