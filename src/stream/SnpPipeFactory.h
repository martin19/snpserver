#ifndef SNPSERVER_SNPPIPEFACTORY_H
#define SNPSERVER_SNPPIPEFACTORY_H

#include <network/snappyv1.pb.h>
#include "SnpComponent.h"
#include <vector>

class SnpPipe;
class SnpClientWebsocket;

class SnpPipeFactory {
public:
    static SnpPipe *createPipe(uint32_t streamId,
                               std::vector<SnpComponent*> components);
private:
};


#endif //SNPSERVER_SNPPIPEFACTORY_H
