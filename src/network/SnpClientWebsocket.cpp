#include "SnpClientWebsocket.h"
#include <utility>
#include <ctime>
#include <stream/SnpPipeFactory.h>
#include <util/PropertyUtil.h>
#include "SnpWebsocket.h"
#include "util/loguru.h"

SnpClientWebsocket::SnpClientWebsocket(SnpWebsocket *server, struct lws *wsi) : server(server), wsi(wsi) {
    connectionStartTs = std::time(nullptr);
}

SnpClientWebsocket::~SnpClientWebsocket() {
    for (auto & it : pipes) {
        auto pipe = it.second;
        pipe->stop();
        for(auto & pComponent : pipe->getComponents()) {
            delete pComponent;
        }
        delete pipe;
    }
}

void SnpClientWebsocket::onMessage(uint8_t *data, int len) {
    //decode message
    snappyv1::Message message = snappyv1::Message();
    message.ParseFromArray(data, len);
    switch(message.type()) {
        case snappyv1::MESSAGE_TYPE_STREAM_CHANGE: {
            onStreamChange(message.stream_change());
        } break;
        case snappyv1::MESSAGE_TYPE_STREAM_DATA: {
            onStreamData(message.stream_data());
        } break;
        default:
            fprintf(stderr, "received unknown message.\n");
    }
}

bool SnpClientWebsocket::operator<(const SnpClientWebsocket &right) const {
    return connectionStartTs < right.connectionStartTs;
}

time_t SnpClientWebsocket::getConnectionStartTs() const {
    return connectionStartTs;
}

void SnpClientWebsocket::onStreamChange(const snappyv1::StreamChange &msg) {
    using namespace snappyv1;
    switch(msg.command()) {
        case COMMAND_INIT: {
            onStreamChangeInit(msg);
        } break;
        case COMMAND_DESTROY: {
            onStreamChangeDestroy(msg);
        } break;
        case COMMAND_START: {
            onStreamChangeStart(msg);
        } break;
        case COMMAND_STOP: {
            onStreamChangeStop(msg);
        } break;
    }
}

void SnpClientWebsocket::onStreamChangeInit(const snappyv1::StreamChange &msg) {
    //find pipe for stream in pipe registry and initialize it. when initialized, send ready.
    auto pipe = SnpPipeFactory::createPipe(msg.id(), this, msg.stream_medium(), msg.stream_direction(),
                                           msg.stream_endpoint(), msg.stream_encoding());
    uint32_t streamId = msg.id();
    if(pipe) {
        pipes.insert(std::pair<uint32_t, SnpPipe*>(streamId, pipe));
        sendStreamChangeInitOk(streamId, pipe);
    }
    /*else {
        sendStreamChangeFail()
    }*/
}

void SnpClientWebsocket::onStreamChangeStart(const snappyv1::StreamChange &msg) {
    auto it = pipes.find(msg.id());
    auto pipe = it->second;
    if(pipe) {
        pipe->start();
    }
}

void SnpClientWebsocket::onStreamChangeStop(const snappyv1::StreamChange &msg) {
    auto it = pipes.find(msg.id());
    auto pipe = it->second;
    if(pipe) {
        pipe->stop();
    }
}

void SnpClientWebsocket::onStreamChangeDestroy(const snappyv1::StreamChange &msg) {
    auto it = pipes.find(msg.id());
    auto pipe = it->second;
    if(pipe) {
        pipe->stop();
        pipes.erase(it);
    }
}

void SnpClientWebsocket::sendStreamChangeInitOk(uint32_t streamId, SnpPipe *pipe) {
   using namespace snappyv1;
   auto *msg1 = new Message();
   msg1->set_type(snappyv1::MESSAGE_TYPE_STREAM_CHANGE);
   auto *streamChange = new StreamChange();
   msg1->set_allocated_stream_change(streamChange);
   streamChange->set_id(streamId);
   streamChange->set_command(COMMAND_INIT_OK);
   streamChange->set_stream_medium(pipe->getMedium());
   streamChange->set_stream_direction(pipe->getDirection());
   streamChange->set_stream_encoding(pipe->getEncoding());
   streamChange->set_stream_endpoint(pipe->getEndpoint());

   //append stream properties.
   auto properties = pipe->getProperties();
   for(auto & pProperty : *properties) {
       auto p = streamChange->add_property();
       PropertyUtil::copySnpPropertyToProtocolProperty(*p, *pProperty);
   }
   delete properties;
   this->server->sendMessage(msg1, wsi);
}

void SnpClientWebsocket::setStreamListener(uint32_t streamId, StreamListener streamListener) {
   streamListeners.insert(std::pair(streamId, streamListener));
}

void SnpClientWebsocket::onStreamData(const snappyv1::StreamData &msg) {
   uint32_t streamId = msg.stream_id();
   auto entry = streamListeners.find(streamId);
   StreamListener listener = entry->second;
   if(listener != nullptr) {
       const auto *data = (const uint8_t*)msg.payload().data();
       int len = msg.payload().length();
       listener(data, len, true);
   }
}

void SnpClientWebsocket::sendStreamData(uint32_t streamId, uint8_t *data, int len) {
   using namespace snappyv1;
   auto *streamData = new StreamData();
   streamData->set_payload(data, len);
   streamData->set_stream_id(streamId);

   //TODO: generalize timing measurement to be used with other encoder decoder chains.
//    if(streamId == 0) {
//        auto *frameTiming = new FrameTiming();
//        frameTiming->set_capture_ts_start_ms(fixedVideoPipe->getComponents().at(0)->getTimestampStartMs());
//        frameTiming->set_capture_ts_end_ms(fixedVideoPipe->getComponents().at(0)->getTimestampEndMs());
//        frameTiming->set_encode_ts_start_ms(fixedVideoPipe->getComponents().at(1)->getTimestampStartMs());
//        frameTiming->set_encode_ts_end_ms(fixedVideoPipe->getComponents().at(1)->getTimestampEndMs());
//        frameTiming->set_send_ts_start_ms(fixedVideoPipe->getComponents().at(2)->getTimestampStartMs());
//        frameTiming->set_send_ts_end_ms(fixedVideoPipe->getComponents().at(2)->getTimestampEndMs());
//        streamData->set_allocated_frame_timing(frameTiming);
//    }

//    if(streamId == 0) {
//        std::cout << "FrameTiming:" << std::endl;
//        std::cout << "Capture duration: " <<
//        (fixedVideoPipe->getComponents().at(0)->getTimestampEndMs() -
//            fixedVideoPipe->getComponents().at(0)->getTimestampStartMs()) << "ms" << std::endl;
//        std::cout << "Encode duration: " <<
//                  (fixedVideoPipe->getComponents().at(1)->getTimestampEndMs() -
//                   fixedVideoPipe->getComponents().at(1)->getTimestampStartMs()) << "ms" << std::endl;
//    }

   auto *msg = new Message();
   msg->set_type(snappyv1::MESSAGE_TYPE_STREAM_DATA);
   msg->set_allocated_stream_data(streamData);

   this->server->sendMessage(msg, wsi);
}





