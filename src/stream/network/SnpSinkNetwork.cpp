#include <util/TimeUtil.h>
#include "SnpSinkNetwork.h"

#define SNP_SINK_NETWORK_BUFFER_SIZE 500000

SnpSinkNetwork::SnpSinkNetwork(const SnpSinkNetworkOptions &options) : SnpComponent(options) {
    componentName = "sinkNetwork";
    streamId = options.streamId;
    client = options.client;

    addInputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkNetwork::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpSinkNetwork::~SnpSinkNetwork() {
    buffer.clear();
}

void SnpSinkNetwork::setEnabled(bool enabled) {
    if(enabled) {
        buffer.reserve(SNP_SINK_NETWORK_BUFFER_SIZE);
        buffer.clear();
    } else {
        buffer.clear();
    }
    SnpComponent::setEnabled(enabled);
}

void SnpSinkNetwork::onInputData(const uint8_t * inputBuffer, int inputLen, bool complete) {
    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
        setTimestampStartMs(TimeUtil::getTimeNowMs());
        this->client->sendStreamData(streamId, buffer.data(), buffer.size());
        setTimestampEndMs(TimeUtil::getTimeNowMs());
        buffer.clear();
        this->getOwner()->framesPassed++;
    }
}
