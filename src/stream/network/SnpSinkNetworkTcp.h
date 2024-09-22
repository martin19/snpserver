#ifndef SNPSERVER_SNPSINKNETWORKTCP_H
#define SNPSERVER_SNPSINKNETWORKTCP_H

#include <stream/SnpComponent.h>
#include <util/TimeUtil.h>
#include <mutex>
#include <condition_variable>
#include "sockets.h"

struct SnpSinkNetworkTcpOptions : public SnpComponentOptions {
    uint32_t streamId;
    std::string host;
    uint16_t port;
};

class SnpSinkNetworkTcp : public SnpComponent {
public:
    explicit SnpSinkNetworkTcp(const SnpSinkNetworkTcpOptions &options);
    ~SnpSinkNetworkTcp() override;

    bool start() override;
    void stop() override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    void createSocket();

    [[noreturn]] void listenForConnections();
    bool sendMessage();
    void destroySocket() const;
    uint32_t streamId;
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
};

#endif //SNPSERVER_SNPSINKNETWORKTCP_H
