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
#include <stream/input/SnpSourceCursor.h>
#include "SnpSocket.h"
#include "util/loguru.h"

SnpClient::SnpClient(SnpSocket *server, struct lws *wsi) : server(server), wsi(wsi) {
    connectionStartTs = std::time(nullptr);
}

SnpClient::~SnpClient() {
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

    if(fixedCursorPipe) {
        fixedCursorPipe->stop();
        for(auto & pComponent : fixedCursorPipe->getComponents()) {
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
        LOG_F(INFO, "creating video pipe.");
        SnpSourceModesettingOptions sourceModesettingOptions;
        sourceModesettingOptions.device = "/dev/dri/card0";
        sourceModesettingOptions.fps = 30;
        auto *sourceModesetting = new SnpSourceModesetting(sourceModesettingOptions);
        ///
        SnpEncoderMmalH264Options encoderMmalH264Options = {};
        encoderMmalH264Options.qp = 20;
        encoderMmalH264Options.width = sourceModesetting->width;
        encoderMmalH264Options.height = sourceModesetting->height;
        encoderMmalH264Options.bpp = sourceModesetting->bpp;
        auto *encoderMmalH264 = new SnpEncoderMmalH264(encoderMmalH264Options);
        ///
        SnpSinkNetworkOptions sinkNetworkOptions = {};
        sinkNetworkOptions.client = this;
        sinkNetworkOptions.streamId = 0;
        auto *sinkNetwork = new SnpSinkNetwork(sinkNetworkOptions);

        SnpPort::connect(sourceModesetting->getOutputPort(0), encoderMmalH264->getInputPort(0));
        SnpPort::connect(encoderMmalH264->getOutputPort(0), sinkNetwork->getInputPort(0));

        SnpPipeOptions videoPipeOptions = {};
        fixedVideoPipe = new SnpPipe(videoPipeOptions);
        fixedVideoPipe->addComponent(sourceModesetting);
        fixedVideoPipe->addComponent(encoderMmalH264);
        fixedVideoPipe->addComponent(sinkNetwork);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Create a fixed mouse pipe (streamId=1)
    {
        LOG_F(INFO, "creating mouse pipe.");
        SnpSourceNetworkOptions sourceNetworkOptions = {};
        sourceNetworkOptions.client = this;
        sourceNetworkOptions.streamId = 1;
        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
        SnpSinkMouseOptions sinkMouseOptions = {};
        sinkMouseOptions.width = 1920;
        sinkMouseOptions.height = 1080;
        auto *snpSinkMouse = new SnpSinkMouse(sinkMouseOptions);
        SnpPort::connect(sourceNetwork->getOutputPort(0), snpSinkMouse->getInputPort(0));

        SnpPipeOptions mousePipeOptions = {};
        fixedMousePipe = new SnpPipe(mousePipeOptions);
        fixedMousePipe->addComponent(sourceNetwork);
        fixedMousePipe->addComponent(snpSinkMouse);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Create a fixed keyboard pipe (streamId=2)
    {
        LOG_F(INFO, "creating keyboard pipe.");
        SnpSourceNetworkOptions sourceNetworkOptions = {};
        sourceNetworkOptions.client = this;
        sourceNetworkOptions.streamId = 2;
        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
        SnpSinkKeyboardOptions sinkKeyboardOptions = {};
        auto *sinkKeyboard = new SnpSinkKeyboard(sinkKeyboardOptions);
        SnpPort::connect(sourceNetwork->getOutputPort(0), sinkKeyboard->getInputPort(0));

        SnpPipeOptions mousePipeOptions = {};
        fixedKeyboardPipe = new SnpPipe(mousePipeOptions);
        fixedKeyboardPipe->addComponent(sourceNetwork);
        fixedKeyboardPipe->addComponent(sinkKeyboard);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //create a fixed cursor pipe (streamId=3)
    {
        LOG_F(INFO, "creating cursor pipe.");
        SnpSourceCursorOptions sourceCursorOptions = {};
        auto *sourceCursor = new SnpSourceCursor(sourceCursorOptions);
        SnpSinkNetworkOptions sinkNetworkOptions = {};
        sinkNetworkOptions.client = this;
        sinkNetworkOptions.streamId = 3;
        auto *sinkNetwork = new SnpSinkNetwork(sinkNetworkOptions);
        SnpPort::connect(sourceCursor->getOutputPort(0), sinkNetwork->getInputPort(0));

        SnpPipeOptions cursorPipeOptions = {};
        fixedCursorPipe = new SnpPipe(cursorPipeOptions);
        fixedCursorPipe->addComponent(sourceCursor);
        fixedCursorPipe->addComponent(sinkNetwork);
    }

    LOG_F(INFO, "Starting pipes.");
    fixedVideoPipe->start();
    fixedMousePipe->start();
//    fixedKeyboardPipe->start();
//    fixedCursorPipe->start();
}

void SnpClient::setStreamListener(uint32_t streamId, StreamListener streamListener) {
    streamListeners.insert(std::pair(streamId, streamListener));
}

void SnpClient::onStreamData(const snappyv1::StreamData &msg) {
    uint32_t streamId = msg.stream_id();
    auto entry = streamListeners.find(streamId);
    StreamListener listener = entry->second;
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
    Message msg;
    msg.set_type(snappyv1::MESSAGE_TYPE_STREAM_DATA);
    msg.set_allocated_stream_data(streamData);
    this->server->sendMessage(&msg, wsi);
}
