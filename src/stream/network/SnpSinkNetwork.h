#ifndef SNPSERVER_SNPSINKNETWORK_H
#define SNPSERVER_SNPSINKNETWORK_H

#include <network/SnpClient.h>

struct SnpSinkNetworkOptions : public SnpComponentOptions {
    uint32_t streamId;
    SnpClient *client;
};

class SnpSinkNetwork : public SnpComponent {
public:
    explicit SnpSinkNetwork(const SnpSinkNetworkOptions &options);
    ~SnpSinkNetwork() override;
private:
    void onInputData(const uint8_t *data, int len, bool complete);
    uint32_t streamId;
    SnpClient *client;
    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSINKNETWORK_H
