#ifndef SNPSERVER_SNPSOURCENETWORK_H
#define SNPSERVER_SNPSOURCENETWORK_H

#include <network/SnpClient.h>
#include "../SnpComponent.h"

struct SnpSourceNetworkOptions : public SnpComponentOptions {
    uint32_t streamId;
    SnpClient *client;
};

class SnpSourceNetwork : public SnpComponent {
public:
    explicit SnpSourceNetwork(const SnpSourceNetworkOptions &options);
    ~SnpSourceNetwork() override;
private:
    void onInputData(const uint8_t *data, int len, bool complete);
    uint32_t streamId;
    SnpClient *client;
    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSOURCENETWORK_H
