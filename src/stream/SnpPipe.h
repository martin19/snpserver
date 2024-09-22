#ifndef SNPSERVER_SNPPIPE_H
#define SNPSERVER_SNPPIPE_H

#include "SnpComponent.h"
#include "SnpProperty.h"
#include "network/snappyv1.pb.h"

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

    snappyv1::StreamMedium getMedium() const;
    void setMedium(snappyv1::StreamMedium medium);
    snappyv1::StreamEndpoint getEndpoint() const;
    void setEndpoint(snappyv1::StreamEndpoint endpoint);
    snappyv1::StreamEncoding getEncoding() const;
    void setEncoding(snappyv1::StreamEncoding encoding);
    snappyv1::StreamDirection getDirection() const;
    void setDirection(snappyv1::StreamDirection direction);

protected:
    std::string componentName;
private:
    std::vector<SnpComponent*> components;
    bool running;

    snappyv1::StreamMedium medium;
    snappyv1::StreamEndpoint endpoint;
    snappyv1::StreamEncoding encoding;
    snappyv1::StreamDirection direction;
};


#endif //SNPSERVER_SNPPIPE_H
