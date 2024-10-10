#ifndef SNPSERVER_SNPPIPEFACTORY_H
#define SNPSERVER_SNPPIPEFACTORY_H

#include <network/snp.pb.h>
#include "SnpComponent.h"
#include "config/SnpConfig.h"
#include <vector>

class SnpPipe;
class SnpClientWebsocket;

class SnpPipeFactory {
public:
    static std::vector<SnpPipe *>* createPipes(SnpConfig *pConfig, std::string side);
    static SnpPipe* createPipe(uint32_t pipeId, const std::vector<snp::Component*>* components);
};


#endif //SNPSERVER_SNPPIPEFACTORY_H
