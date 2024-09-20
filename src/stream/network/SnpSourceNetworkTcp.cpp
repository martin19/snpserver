#include <mutex>
#include "SnpSourceNetworkTcp.h"
#include "network/snappyv1.pb.h"

#define SNP_SOURCE_RECV_BUFFER_SIZE 500000
#define SNP_SOURCE_NETWORK_BUFFER_SIZE 500000

SnpSourceNetworkTcp::SnpSourceNetworkTcp(const SnpSourceNetworkTcpOptions &options) : SnpComponent(options, "sourceNetwork") {
    streamId = options.streamId;
    host = options.host;
    port = options.port;

    addOutputPort(new SnpPort());
}

SnpSourceNetworkTcp::~SnpSourceNetworkTcp() {
    buffer.clear();
}

void SnpSourceNetworkTcp::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
    if(enabled) {
        buffer.reserve(SNP_SOURCE_NETWORK_BUFFER_SIZE);
        buffer.clear();
    } else {
        buffer.clear();
    }
}

bool SnpSourceNetworkTcp::start() {
    SnpComponent::start();
    createSocket();
    return true;
}

void SnpSourceNetworkTcp::stop() {
    SnpComponent::stop();
    destroySocket();
}

void SnpSourceNetworkTcp::createSocket() {
    sock_init();
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0) {
        LOG_F(ERROR, "failed to create socket (error=%s)", strerror(errno));
        return;
    }
    LOG_F(INFO, "socket created successfully.");

    connectThread = std::thread(&SnpSourceNetworkTcp::connectToServer, this);
}

[[noreturn]] void SnpSourceNetworkTcp::connectToServer() {
    uint8_t recvBuffer[SNP_SOURCE_RECV_BUFFER_SIZE];
    SnpPort *outputPort = getOutputPort(0);
    while(true) {

        {
            std::unique_lock<std::mutex> lock(mutex);
            conditionVariable.wait(lock, [this]{ return !connected; });
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(host.c_str());
        int result = connect(clientSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(result == SOCKET_ERROR) {
            LOG_F(ERROR, "failed to connect to server (error=%s)", strerror(errno));
            connected = false;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
        }
        LOG_F(INFO, "connected to server %s:%d", host.c_str(), port);
        connected = true;

        while(connected) {
            ssize_t bytes_received = recv(clientSocket, (char*)recvBuffer, sizeof(recvBuffer), 0);
            if(bytes_received == -1) {
                LOG_F(ERROR, "failed to read from socket (error=%s)", strerror(errno));
                connected = false;
                continue;
            }
            buffer.insert(buffer.end(), recvBuffer, recvBuffer + bytes_received);
            result = dispatch();
            if(result) {
                buffer.clear();
            }
        }
    }
}

bool SnpSourceNetworkTcp::dispatch() {
    SnpPort *outputPort = getOutputPort(0);
    snappyv1::Message message;
    snappyv1::StreamData streamData;
    bool result = message.ParseFromArray(buffer.data(), (int)buffer.size());
    if(!result) return false;
    switch(message.type()) {
        case snappyv1::MESSAGE_TYPE_STREAM_CHANGE:
            LOG_F(WARNING, "MESSAGE_TYPE_STREAM_CHANGE not implemented, skipping.");
            return true;
        case snappyv1::MESSAGE_TYPE_STREAM_DATA:
            outputPort->onData((const uint8_t*)streamData.payload().c_str(), streamData.payload().size(),
                               true);
            return true;
        case snappyv1::MESSAGE_TYPE_STREAM_INFO:
            LOG_F(WARNING, "MESSAGE_TYPE_STREAM_INFO not implemented, skipping.");
            return true;
    }
    return true;
}

void SnpSourceNetworkTcp::destroySocket() const {
    closesocket(clientSocket);
    sock_cleanup();
}