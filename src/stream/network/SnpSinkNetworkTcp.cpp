#include "SnpSinkNetworkTcp.h"
#include "sockets.h"
#include "util/loguru.h"
#include "network/snp.pb.h"
#include "stream/SnpComponentRegistry.h"

#define SNP_SINK_NETWORK_BUFFER_SIZE 500000

SnpSinkNetworkTcp::SnpSinkNetworkTcp(const SnpSinkNetworkTcpOptions &options) : SnpComponent(options, "COMPONENT_OUTPUT_TCP") {
    host = options.host;
    port = options.port;
    clientConnected = false;

    addInputPort(new SnpPort());
    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkNetworkTcp::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

SnpSinkNetworkTcp::~SnpSinkNetworkTcp() {
    buffer.clear();
}

bool SnpSinkNetworkTcp::start() {
    SnpComponent::start();
    buffer.reserve(SNP_SINK_NETWORK_BUFFER_SIZE);
    buffer.clear();
    createSocket();
    return true;
}

void SnpSinkNetworkTcp::stop() {
    SnpComponent::stop();
    buffer.clear();
    destroySocket();
}

void SnpSinkNetworkTcp::onInputData(uint32_t pipeId, const uint8_t * inputBuffer, int inputLen, bool complete) {
    if(!clientConnected) {
        return;
    }

    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
        setTimestampStartMs(TimeUtil::getTimeNowMs());
        sendDataMessage(pipeId);
        setTimestampEndMs(TimeUtil::getTimeNowMs());
        buffer.clear();
    }
}

void SnpSinkNetworkTcp::createSocket() {
    sock_init();
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket < 0) {
        LOG_F(ERROR, "failed to create socket (error=%s)", strerror(errno));
        return;
    }
    LOG_F(INFO, "socket created successfully.");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());

    int result = bind(listenSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(result == SOCKET_ERROR) {
        LOG_F(ERROR, "failed to bind socket (error=%s)", strerror(errno));
        destroySocket();
        return;
    }
    LOG_F(INFO, "socket bind() successful.");

    result = listen(listenSocket, SOMAXCONN);
    if(result == SOCKET_ERROR) {
        LOG_F(ERROR, "failed to listen on socket (error=%s)", strerror(errno));
        destroySocket();
        return;
    }
    LOG_F(INFO, "socket listening for incoming connections...");

    listenThread = std::thread(&SnpSinkNetworkTcp::listenForConnections, this);
}

[[noreturn]] void SnpSinkNetworkTcp::listenForConnections() {
    while(true) {

        {
            std::unique_lock<std::mutex> lock(mutex);
            conditionVariable.wait(lock, [this]{ return !clientConnected; });
        }

        clientSocket = accept(listenSocket, nullptr, nullptr);
        if(clientSocket == INVALID_SOCKET) {
            LOG_F(ERROR, "failed to accept connection (error=%s)", strerror(errno));
            destroySocket();
            clientConnected = false;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
        }
        LOG_F(INFO, "client connected.");
        clientConnected = true;
        sendCapabilitiesMessage();
    }
}

void SnpSinkNetworkTcp::destroySocket() const {
    closesocket(listenSocket);
    sock_cleanup();
    //TODO: how to stop listen thread and clean it up?
}

bool SnpSinkNetworkTcp::sendDataMessage(uint32_t pipeId) {
    snp::Message message;
    message.set_type(snp::MESSAGE_TYPE_DATA);
    auto data = message.data();
    auto dataRaw = data.dataraw();
    std::basic_string<char> payloadString((const char *)(buffer.data()), buffer.size());
    dataRaw.set_payload(payloadString);
    data.set_pipe_id(pipeId);

    //send message on socket
    std::basic_string<char> messageString = message.SerializeAsString();
    int result = send(clientSocket, messageString.c_str(), (int)messageString.size(), 0);
    LOG_F(INFO, "sent data len=%d", buffer.size());
    if(result == SOCKET_ERROR) {
        LOG_F(ERROR, "failed to write to socket (error=%s)", strerror(errno));
        clientConnected = false;
        return false;
    }
    return true;
}

bool SnpSinkNetworkTcp::sendCapabilitiesMessage() {
    snp::Message message;
    message.set_type(snp::MESSAGE_TYPE_CAPABILITIES);
    auto capabilities = message.mutable_capabilities();
    capabilities->set_platform(snp::PLATFORM_WINDOWS);
    SnpComponentRegistry snpComponentRegistry;
    auto registeredComponents = snpComponentRegistry.getLocalComponents();
    for (const auto &registeredComponent: registeredComponents) {
        auto component = capabilities->add_component();
        component->set_componenttype(registeredComponent->componenttype());
    }

    //send message on socket
    std::basic_string<char> messageString = message.SerializeAsString();
    int result = send(clientSocket, messageString.c_str(), (int)messageString.size(), 0);
    LOG_F(INFO, "sent data len=%d", buffer.size());
    if(result == SOCKET_ERROR) {
        LOG_F(ERROR, "failed to write to socket (error=%s)", strerror(errno));
        clientConnected = false;
        return false;
    }
    return true;
}