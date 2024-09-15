#include <util/TimeUtil.h>
#include "SnpSinkNetworkWebsocket.h"

#define SNP_SINK_NETWORK_BUFFER_SIZE 500000

SnpSinkNetworkWebsocket::SnpSinkNetworkWebsocket(const SnpSinkNetworkOptions &options) : SnpComponent(options, "sinkNetwork") {
    streamId = options.streamId;
    //TODO: websocket client = options.client;

    addInputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkNetworkWebsocket::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpSinkNetworkWebsocket::~SnpSinkNetworkWebsocket() {
    buffer.clear();
}

void SnpSinkNetworkWebsocket::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
    if(enabled) {
        buffer.reserve(SNP_SINK_NETWORK_BUFFER_SIZE);
        buffer.clear();
    } else {
        buffer.clear();
    }
}

void SnpSinkNetworkWebsocket::onInputData(const uint8_t * inputBuffer, int inputLen, bool complete) {
    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
        setTimestampStartMs(TimeUtil::getTimeNowMs());
        //TODO: websocket this->client->sendStreamData(streamId, buffer.data(), buffer.size());
        setTimestampEndMs(TimeUtil::getTimeNowMs());
        buffer.clear();
        //TODO: websocket this->getOwner()->framesPassed++;
    }
}
