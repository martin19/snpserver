#ifndef SNPSERVER_SNPSINKNETWORKWEBSOCKET_H
#define SNPSERVER_SNPSINKNETWORKWEBSOCKET_H

#include <network/SnpClientWebsocket.h>
#include <stream/SnpComponent.h>

struct SnpSinkNetworkOptions : public SnpComponentOptions {
    uint32_t streamId;
    //SnpClientWebsocket *client;
};

class SnpSinkNetworkWebsocket : public SnpComponent {
public:
    explicit SnpSinkNetworkWebsocket(const SnpSinkNetworkOptions &options);
    ~SnpSinkNetworkWebsocket() override;

    bool start() override;
    void stop() override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    uint32_t streamId;
    //SnpClientWebsocket *client;
    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSINKNETWORKWEBSOCKET_H
