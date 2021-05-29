#include "SnpClient.h"
#include <utility>
#include <ctime>
#include <stream/network/SnpSinkNetwork.h>
#include <stream/video/SnpSourceModesetting.h>
#include <stream/video/SnpEncoderMmalH264.h>
#include <stream/network/SnpSinkNetwork.h>
#include <stream/network/SnpSourceNetwork.h>
#include <stream/input/SnpSinkMouse.h>
#include <stream/input/SnpSinkKeyboard.h>
#include "SnpSocket.h"

SnpClient::SnpClient(SnpSocket *server, struct lws *wsi) : server(server), wsi(wsi) {
    connectionStartTs = std::time(nullptr);
}

SnpClient::~SnpClient() {
    //TODO: cleanup fixed pipes
    if(fixedVideoPipe) {
        fixedVideoPipe->stop();
        for(auto & pComponent : fixedVideoPipe->getComponents()) {
            delete pComponent;
        }
    }

    if(fixedMousePipe) {
        fixedMousePipe->stop();
        for(auto & pComponent : fixedMousePipe->getComponents()) {
            delete pComponent;
        }
    }

    if(fixedKeyboardPipe) {
        fixedKeyboardPipe->stop();
        for(auto & pComponent : fixedKeyboardPipe->getComponents()) {
            delete pComponent;
        }
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Create a fixed video pipe
    {
        SnpSourceModesettingOptions snpSourceModesettingOptions;
        snpSourceModesettingOptions.device = "/dev/dri/card0";
        snpSourceModesettingOptions.fps = 30;
        auto *snpSourceModesetting = new SnpSourceModesetting(snpSourceModesettingOptions);
        ///
        SnpEncoderMmalH264Options snpEncoderMmalH264Options = {};
        snpEncoderMmalH264Options.qp = 20;
        auto *snpEncoderMmalH264 = new SnpEncoderMmalH264(snpEncoderMmalH264Options);
        ///
        SnpSinkNetworkOptions snpSinkNetworkOptions = {};
        snpSinkNetworkOptions.client = this;
        snpSinkNetworkOptions.streamId = 0;
        auto *snpSinkNetwork = new SnpSinkNetwork(snpSinkNetworkOptions);

        SnpPort::connect(snpSourceModesetting->getOutput(0), snpEncoderMmalH264->getInput(0));
        SnpPort::connect(snpEncoderMmalH264->getOutput(0), snpSinkNetwork->getInput(0));

        SnpPipeOptions videoPipeOptions = {};
        fixedVideoPipe = new SnpPipe(videoPipeOptions);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Create a fixed mouse pipe (streamId=1)
    {
        SnpSourceNetworkOptions snpSourceNetworkOptions = {};
        snpSourceNetworkOptions.client = this;
        snpSourceNetworkOptions.streamId = 1;
        auto *snpSourceNetwork = new SnpSourceNetwork(snpSourceNetworkOptions);
        SnpSinkMouseOptions snpSinkMouseOptions = {};
        auto *snpSinkMouse = new SnpSinkMouse(snpSinkMouseOptions);
        SnpPort::connect(snpSourceNetwork->getOutput(0), snpSinkMouse->getInput(0));

        SnpPipeOptions mousePipeOptions = {};
        fixedMousePipe = new SnpPipe(mousePipeOptions);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Create a fixed keyboard pipe (streamId=2)
    {
        SnpSourceNetworkOptions snpSourceNetworkOptions = {};
        snpSourceNetworkOptions.client = this;
        snpSourceNetworkOptions.streamId = 2;
        auto *snpSourceNetwork = new SnpSourceNetwork(snpSourceNetworkOptions);
        SnpSinkKeyboardOptions snpSinkKeyboardOptions = {};
        auto *snpSinkKeyboard = new SnpSinkKeyboard(snpSinkKeyboardOptions);
        SnpPort::connect(snpSourceNetwork->getOutput(0), snpSinkKeyboard->getInput(0));

        SnpPipeOptions mousePipeOptions = {};
        fixedKeyboardPipe = new SnpPipe(mousePipeOptions);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    fixedVideoPipe->start();
    fixedMousePipe->start();
    fixedKeyboardPipe->start();
    //TODO: kill pipes.
}

void SnpClient::setStreamListener(uint32_t streamId, StreamListener streamListener) {
    streamListeners.insert(std::pair(streamId, streamListener));
}

void SnpClient::onStreamData(const snappyv1::StreamData &msg) {
    uint32_t streamId = msg.stream_id();
    StreamListener listener = streamListeners.at(streamId);
    if(listener != nullptr) {
        const auto *data = (const uint8_t*)msg.payload().data();
        int len = msg.payload().length();
        listener(data, len, true);
    }
}

void SnpClient::sendStreamData(uint32_t streamId, uint8_t *data, int len) {
    using namespace snappyv1;
    auto *streamData = new StreamData();
    streamData->set_payload(data, len);
    streamData->set_stream_id(streamId);
    auto msg = new Message();
    msg->set_type(snappyv1::MESSAGE_TYPE_STREAM_DATA);
    msg->set_allocated_stream_data(streamData);
    this->server->sendMessage(msg, wsi);
    //TODO: msg should be freed or created as local variable? lookup protobuf docs
}
