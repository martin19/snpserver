#ifndef SNPSERVER_SNPPIPEFACTORY_H
#define SNPSERVER_SNPPIPEFACTORY_H

#include <network/snp.pb.h>
#include "SnpComponent.h"
#include <vector>

class SnpPipe;
class SnpClientWebsocket;
typedef std::map<uint32_t, std::vector<snp::Component*>> PipeMap;

class SnpPipeFactory {
public:
    static SnpPipe* createPipe(uint32_t pipeId, const std::vector<snp::Component*>& components);
    static std::vector<SnpPipe *> createPipes(PipeMap& pipeMap);
};


#endif //SNPSERVER_SNPPIPEFACTORY_H
