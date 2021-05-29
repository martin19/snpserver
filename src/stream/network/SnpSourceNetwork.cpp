#include "SnpSourceNetwork.h"

SnpSourceNetwork::SnpSourceNetwork(const SnpSourceNetworkOptions &options) : SnpComponent(options) {
    this->client = options.client;
    this->streamId = options.streamId;

    addOutput(new SnpPort());

    client->setStreamListener(streamId, getOutput(0)->onDataCb);
}

SnpSourceNetwork::~SnpSourceNetwork() {

}

void SnpSourceNetwork::onInputData(uint8_t *data, int len, bool complete) {
    getOutput(0)->onDataCb(data, len, complete);
}
