#ifndef SNPSERVER_SNPSINKNETWORK_H
#define SNPSERVER_SNPSINKNETWORK_H

//#include <network/SnpClientWebsocket.h>
#include <stream/SnpComponent.h>

struct SnpSinkNetworkOptions : public SnpComponentOptions {
    uint32_t streamId;
    //SnpClientWebsocket *client;
};

class SnpSinkNetwork : public SnpComponent {
public:
    explicit SnpSinkNetwork(const SnpSinkNetworkOptions &options);
    ~SnpSinkNetwork() override;

    void setEnabled(bool enabled) override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    uint32_t streamId;
    //SnpClientWebsocket *client;
    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSINKNETWORK_H
