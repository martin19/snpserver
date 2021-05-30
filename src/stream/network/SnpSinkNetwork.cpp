#include "SnpSinkNetwork.h"

#define SNP_SINK_NETWORK_BUFFER_SIZE 500000

SnpSinkNetwork::SnpSinkNetwork(const SnpSinkNetworkOptions &options) : SnpComponent(options) {
    streamId = options.streamId;
    client = options.client;

    addInputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkNetwork::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    buffer.reserve(SNP_SINK_NETWORK_BUFFER_SIZE);
}

SnpSinkNetwork::~SnpSinkNetwork() {
}

void SnpSinkNetwork::onInputData(const uint8_t * inputBuffer, int inputLen, bool complete) {
    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
//        usleep(16666);
        this->client->sendStreamData(streamId, buffer.data(), buffer.size());
    }
    buffer.clear();
}
