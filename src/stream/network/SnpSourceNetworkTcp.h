#ifndef SNPSERVER_SNPSOURCENETWORKTCP_H
#define SNPSERVER_SNPSOURCENETWORKTCP_H

#include "../SnpComponent.h"
#include <mutex>
#include <condition_variable>
#include "sockets.h"
#include "network/snp.pb.h"
#include "config/SnpConfig.h"

typedef void (*HandleCapabilitiesMessageCb)(snp::Message* message);

struct SnpSourceNetworkTcpOptions : public SnpComponentOptions {
    std::string host;
    uint16_t port;
    std::vector<PortStreamType> portStreamTypes;
    HandleCapabilitiesMessageCb handleCapabilitiesMessageCb;
};

class SnpSourceNetworkTcp : public SnpComponent {
public:
    explicit SnpSourceNetworkTcp(const SnpSourceNetworkTcpOptions &options);
    ~SnpSourceNetworkTcp() override;
    bool start() override;
    void stop() override;
    bool sendSetupMessage(SnpConfig *pConfig);

private:
    void createSocket();
    void destroySocket() const;

    [[noreturn]] void connectToServer();
    bool dispatch();
    HandleCapabilitiesMessageCb handleCapabilitiesMessageCb;

    SOCKET clientSocket;
    std::thread connectThread;

    bool connected;
    std::mutex mutex;
    std::condition_variable conditionVariable;

    uint16_t port;
    std::string host;
    struct sockaddr_in server_addr;

    std::vector<uint8_t> buffer;
};

#endif //SNPSERVER_SNPSOURCENETWORKTCP_H
