#ifndef SNPSERVER_SNPSOURCENETWORKTCP_H
#define SNPSERVER_SNPSOURCENETWORKTCP_H

#include "../SnpComponent.h"
#include <mutex>
#include <condition_variable>
#include "sockets.h"
#include "network/snappyv1.pb.h"

typedef void (*HandleCapabilitiesMessageCb)(snappyv1::Message* message);

struct SnpSourceNetworkTcpOptions : public SnpComponentOptions {
    uint32_t streamId;
    std::string host;
    uint16_t port;
    HandleCapabilitiesMessageCb handleCapabilitiesMessageCb;
};

class SnpSourceNetworkTcp : public SnpComponent {
public:
    explicit SnpSourceNetworkTcp(const SnpSourceNetworkTcpOptions &options);
    ~SnpSourceNetworkTcp() override;

    bool start() override;
    void stop() override;

private:
    void createSocket();
    void destroySocket() const;

    [[noreturn]] void connectToServer();
    bool dispatch();
    HandleCapabilitiesMessageCb handleCapabilitiesMessageCb;

    uint32_t streamId;

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
