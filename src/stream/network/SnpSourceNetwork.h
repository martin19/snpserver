#ifndef SNPSERVER_SNPSOURCENETWORK_H
#define SNPSERVER_SNPSOURCENETWORK_H

#include <network/SnpClient.h>
#include "../SnpSource.h"

struct SnpSourceNetworkOptions : public SnpSourceOptions {
    SnpClient *client;
};

class SnpSourceNetwork : public SnpSource {
public:
    explicit SnpSourceNetwork(const SnpSourceNetworkOptions &options);
    ~SnpSourceNetwork() override;
private:
    SnpClient *client;
    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSOURCENETWORK_H
