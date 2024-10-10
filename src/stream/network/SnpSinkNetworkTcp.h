#ifndef SNPSERVER_SNPSINKNETWORKTCP_H
#define SNPSERVER_SNPSINKNETWORKTCP_H

#include <stream/SnpComponent.h>
#include <util/TimeUtil.h>
#include <mutex>
#include <condition_variable>
#include "sockets.h"
#include "network/snp.pb.h"

typedef void (*HandleSetupMessageCb)(snp::Message* message);

struct SnpSinkNetworkTcpOptions : public SnpComponentOptions {
    std::string host;
    uint16_t port;
    std::vector<PortStreamType> portStreamTypes;
    HandleSetupMessageCb handleSetupMessageCb;
};

class SnpSinkNetworkTcp : public SnpComponent {
public:
    explicit SnpSinkNetworkTcp(const SnpSinkNetworkTcpOptions &options);
    ~SnpSinkNetworkTcp() override;

    bool start() override;
    void stop() override;

private:
    void onInputData(uint32_t pipeId, const uint8_t *data, int len, bool complete);
    void createSocket();

    [[noreturn]] void listenForConnections();
    bool sendDataMessage(uint32_t pipeId);
    bool sendCapabilitiesMessage();
    void destroySocket() const;
    bool dispatch();

    std::vector<uint8_t> buffer;

    SOCKET listenSocket;
    SOCKET clientSocket;
    std::thread listenThread;

    bool clientConnected;
    std::mutex mutex;
    std::condition_variable conditionVariable;

    uint16_t port;
    std::string host;
    struct sockaddr_in server_addr;

    HandleSetupMessageCb handleSetupMessageCb;
};

#endif //SNPSERVER_SNPSINKNETWORKTCP_H
