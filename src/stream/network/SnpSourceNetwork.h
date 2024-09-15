#ifndef SNPSERVER_SNPSOURCENETWORK_H
#define SNPSERVER_SNPSOURCENETWORK_H

//#include <network/SnpClientWebsocket.h>
#include "../SnpComponent.h"

struct SnpSourceNetworkOptions : public SnpComponentOptions {
    uint32_t streamId;
    //TODO: SnpClientWebsocket *client;
};

class SnpSourceNetwork : public SnpComponent {
public:
    explicit SnpSourceNetwork(const SnpSourceNetworkOptions &options);
    ~SnpSourceNetwork() override;

    void setEnabled(bool enabled) override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    uint32_t streamId;
    //SnpClientWebsocket *client;
    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSOURCENETWORK_H
