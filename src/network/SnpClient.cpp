#include "SnpClient.h"
#include <utility>
#include <ctime>
#include <stream/SnpSinkNetwork.h>
#include <stream/SnpSourceModesetting.h>
#include <stream/SnpEncoderMmalH264.h>
#include "SnpSocket.h"

SnpClient::SnpClient(SnpSocket *server, const websocketpp::connection_hdl &hdl) : server(server), hdl(hdl) {
    connectionStartTs = std::time(nullptr);
}

SnpClient::~SnpClient() {
    if(fixedVideoPipeline) {
        fixedVideoPipeline->stop();
        delete fixedVideoPipeline->getSink();
        delete fixedVideoPipeline->getEncoder();
        delete fixedVideoPipeline->getSource();
        delete fixedVideoPipeline;
    }
}

void SnpClient::onMessage(uint8_t *data, int len) {
    //decode message
    printf("got a message len=%d\n",len);
    snappyv1::Message message = snappyv1::Message();
    message.ParseFromArray(data, len);
    switch(message.type()) {
        case snappyv1::MESSAGE_TYPE_STREAMS_CHANGE: {
            onStreamsChange(message.stream_change());
        } break;
        case snappyv1::MESSAGE_TYPE_STREAM_DATA: {
            onStreamData(message.stream_data());
        } break;
        default:
            fprintf(stderr, "received unknown message.\n");
    }
}

void SnpClient::send(uint8_t *buffer, int len) {
    server->send(hdl, buffer, len);
}

void SnpClient::send(std::string &message) {
    server->send(hdl, message);
}

bool SnpClient::operator<(const SnpClient &right) const {
    return connectionStartTs < right.connectionStartTs;
}

time_t SnpClient::getConnectionStartTs() const {
    return connectionStartTs;
}

void SnpClient::onStreamsChange(const snappyv1::StreamsChange &msg) {
    printf("got StreamsChange message\n");
    //TODO: Negotiation: determine what stream was requested by client and initialize an appropriate stream.
    //  For now: create one fixed h264 stream for testing.

    //TODO: more general: define "ports" with "types" and "connect" them
    SnpSourceModesettingOptions snpSourceModesettingOptions;
    snpSourceModesettingOptions.fps = 30;
    snpSourceModesettingOptions.sourceType = SOURCE_TYPE_MMAP;
    snpSourceModesettingOptions.device = "/dev/dri/card0";
    auto *snpSourceModesetting = new SnpSourceModesetting(snpSourceModesettingOptions);
    ///
    SnpEncoderMmalH264Options snpEncoderMmalH264Options;
    snpEncoderMmalH264Options.qp = 20;
    snpEncoderMmalH264Options.inputDescriptor = snpSourceModesetting->getOutputDescriptor();
    auto *snpEncoderMmalH264 = new SnpEncoderMmalH264(snpEncoderMmalH264Options);
    ///
    SnpSinkNetworkOptions snpSinkNetworkOptions = {};
    snpSinkNetworkOptions.client = this;
    //snpSinkNetworkOptions.inputDescriptor = snpEncoderMmalH264.getOutputDescriptor();
    auto *snpSinkNetwork = new SnpSinkNetwork(snpSinkNetworkOptions);

    SnpPipelineOptions pipelineOptions;
    pipelineOptions.source = snpSourceModesetting;
    pipelineOptions.encoder = snpEncoderMmalH264;
    pipelineOptions.sink = snpSinkNetwork;
    fixedVideoPipeline = new SnpPipeline(pipelineOptions);
    printf("starting h264 pipeline\n");
    fixedVideoPipeline->start();
}

void SnpClient::onStreamData(const snappyv1::StreamData &msg) {

}

void SnpClient::sendStreamData(uint8_t *data, int len) {
    auto *streamData = new snappyv1::StreamData();
    streamData->set_payload(data, len);
    streamData->set_stream_id(0);
    snappyv1::Message msg = snappyv1::Message();
    msg.set_type(snappyv1::MESSAGE_TYPE_STREAM_DATA);
    msg.set_allocated_stream_data(streamData);
    std::string s = msg.SerializeAsString();
    send(s);
}