#include "SnpSourceNetwork.h"

SnpSourceNetwork::SnpSourceNetwork(const SnpSourceNetworkOptions &options) : SnpComponent(options, "sourceNetwork") {
    this->client = options.client;
    this->streamId = options.streamId;

    addOutputPort(new SnpPort());

    client->setStreamListener(streamId, std::bind(&SnpSourceNetwork::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpSourceNetwork::~SnpSourceNetwork() {

}

void SnpSourceNetwork::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
}

void SnpSourceNetwork::onInputData(const uint8_t *data, int len, bool complete) {
    getOutputPort(0)->onData(data, len, complete);
}