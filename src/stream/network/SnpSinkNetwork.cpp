#include "SnpSinkNetwork.h"

#define SNP_SINK_NETWORK_BUFFER_SIZE 500000

SnpSinkNetwork::SnpSinkNetwork(const SnpSinkNetworkOptions &options) : SnpSink(options) {
    this->client = options.client;
    buffer.reserve(SNP_SINK_NETWORK_BUFFER_SIZE);
}

SnpSinkNetwork::~SnpSinkNetwork() {
}

void SnpSinkNetwork::process(uint8_t * inputBuffer, int inputLen, bool complete) {
    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
//        usleep(16666);
        this->client->sendStreamData(buffer.data(), buffer.size());
    }
    buffer.clear();
}
