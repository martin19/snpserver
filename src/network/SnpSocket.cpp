#include "SnpSocket.h"

#include "libwebsockets.h"
#include "snappyv1.pb.h"
#include "util/loguru.h"

static struct lws_protocols protocols[] = {
    { "http", SnpSocket::callback_http, 0 },
    { NULL, NULL, 0, 0 } /* terminator */
};

SnpSocket::SnpSocket() {
    memset(&info, 0, sizeof info);
    info.port = 9002;
    info.protocols = protocols;
    info.pt_serv_buf_size = 32 * 1024;
    info.user = this;
    context = lws_create_context(&info);
    sendBufferLen = 0;
}

SnpSocket::~SnpSocket() {
    lws_context_destroy(context);
}

void SnpSocket::run() {
    int n = 0;
    while (n >= 0 /* && !interrupted*/)
        n = lws_service(context, 0);
}


int SnpSocket::callback_http(struct lws *wsi,
                         lws_callback_reasons reason,
                         void *user, void *in, size_t len) {

    SnpSocket* self = nullptr;
    if(wsi != nullptr) {
        lws_context *context = lws_get_context(wsi);
        self = reinterpret_cast<SnpSocket*>(lws_context_user(context));
    }

//    struct per_session_data__minimal_server_echo *pss =
//        (struct per_session_data__minimal_server_echo *)user;
    struct vhd_minimal_server_echo *vhd = (struct vhd_minimal_server_echo *)
        lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                 lws_get_protocol(wsi));
    const struct msg *pmsg;
//    struct msg amsg;

    switch(reason) {
        case LWS_CALLBACK_PROTOCOL_INIT: {
            LOG_F(INFO,"LWS_CALLBACK_PROTOCOL_INIT");
        } break;
        case LWS_CALLBACK_WSI_CREATE: {
            LOG_F(INFO,"LWS_CALLBACK_WSI_CREATE");
        } break;
        case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE: {
            LOG_F(INFO,"LWS_CALLBACK_HTTP_CONFIRM_UPGRADE");
        } break;
        case LWS_CALLBACK_HTTP_BIND_PROTOCOL: {
            LOG_F(INFO,"LWS_CALLBACK_HTTP_BIND_PROTOCOL");
        } break;
        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: {
//            printf("LWS_CALLBACK_EVENT_WAIT_CANCELLED\n");
        } break;
        case LWS_CALLBACK_ADD_HEADERS: {
            LOG_F(INFO,"LWS_CALLBACK_ADD_HEADERS");
        } break;
        case LWS_CALLBACK_ESTABLISHED: {
            LOG_F(INFO,"LWS_CALLBACK_ESTABLISHED\n");
            if(self != nullptr) {
                auto *client = new SnpClient(self, wsi);
                self->clients.insert(std::pair(wsi, client));
                self->sendStreamInfo(wsi);
            }
        } break;
        case LWS_CALLBACK_RECEIVE: {
//            printf("LWS_CALLBACK_RECEIVE\n");

            int first = lws_is_first_fragment(wsi);
            int final = lws_is_final_fragment(wsi);
            int binary = lws_frame_is_binary(wsi);

            //TODO: use logging facility, looks nice
//            printf("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, last %d, bin %d)\n",
//                      (int)len, (int)lws_remaining_packet_payload(wsi),
//                      first, final, binary);

            if (len) {
//                lwsl_hexdump_notice(in, len);
            }

            if(first && final && binary) {
                self->onMessage(wsi, (uint8_t*)in, len);
            } else {
                LOG_F(ERROR,"Did not receive a full binary message, packet is fragemented - cannot handle fragmented packets yet.");
            }
        } break;
        case LWS_CALLBACK_CLOSED: {
            //TODO: remove client pointer and clean up.
            LOG_F(INFO,"LWS_CALLBACK_CLOSED");
        } break;
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
            LOG_F(INFO,"LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
        } break;
        case LWS_CALLBACK_WSI_DESTROY: {
            LOG_F(INFO,"LWS_CALLBACK_WSI_DESTROY");
        } break;
        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: {
            LOG_F(INFO,"LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED");
        } break;
        case LWS_CALLBACK_CLOSED_HTTP: {
            LOG_F(INFO,"LWS_CALLBACK_CLOSED_HTTP");
        } break;
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: {
            LOG_F(INFO,"LWS_CALLBACK_FILTER_NETWORK_CONNECTION");
        } break;
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: {
            LOG_F(INFO,"LWS_CALLBACK_WS_PEER_INITIATED_CLOSE");
        } break;
        //case LWS_CALLBACK_CONNECTING: {
        //    LOG_F(INFO,"LWS_CALLBACK_CONNECTING");
        //} break;
        case LWS_CALLBACK_PROTOCOL_DESTROY: {
            LOG_F(INFO,"LWS_CALLBACK_PROTOCOL_DESTROY");
        } break;
        case LWS_CALLBACK_SERVER_WRITEABLE: {
//            printf("LWS_CALLBACK_SERVER_WRITEABLE\n");
//            std::cout << "Output queue size = " << self->outputQueue.size() << std::endl;
            if(self && !self->outputQueue.empty()) {
                //send next message.
                snappyv1::Message *msg = self->outputQueue.front();
                self->sendBufferLen = msg->ByteSizeLong();
                msg->SerializeToArray(&self->sendBuffer[LWS_PRE], self->sendBufferLen);
                //TODO: assuming whole message can be sent.
                lws_write(wsi, &self->sendBuffer[LWS_PRE], self->sendBufferLen, LWS_WRITE_BINARY);
                self->sendBufferLen = 0;
                self->outputQueue.pop();
                delete msg;
            }

            if(!self->outputQueue.empty()) {
                lws_callback_on_writable(wsi);
            }
        } break;
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            LOG_F(INFO,"LWS_CALLBACK_CLIENT_WRITEABLE");
        } break;
        case LWS_CALLBACK_HTTP: {
            LOG_F(INFO,"LWS_CALLBACK_HTTP");
        } break;
        default: {
            LOG_F(INFO,"unhandled callback %d", reason);
        }
    }
    return 0;
}

void SnpSocket::sendMessage(snappyv1::Message *msg, struct lws *wsi) {
    outputQueue.push(msg);
    lws_callback_on_writable(wsi);
}

void SnpSocket::sendStreamInfo(lws* wsi) {
    LOG_F(INFO,"sendStreamInfo\n");

    using namespace snappyv1;
    auto pStreamInfo = new StreamInfo();
    pStreamInfo->set_platform(PLATFORM_RASPBERRY);

    pStreamInfo->add_stream_endpoints(STREAM_ENDPOINT_POINTER);
    pStreamInfo->add_stream_endpoints(STREAM_ENDPOINT_KEYBOARD);
    pStreamInfo->add_stream_endpoints(STREAM_ENDPOINT_X11);
    pStreamInfo->add_stream_endpoints(STREAM_ENDPOINT_CURSOR);

    pStreamInfo->add_stream_encodings(STREAM_ENCODING_H264_HARDWARE);
    pStreamInfo->add_stream_encodings(STREAM_ENCODING_H264_SOFTWARE);

    //wrap server info message in envelope
    auto *msgEnvelope = new Message();
    msgEnvelope->set_type(MESSAGE_TYPE_STREAM_INFO);
    msgEnvelope->set_allocated_stream_info(pStreamInfo);

    sendMessage(msgEnvelope, wsi);
}

void SnpSocket::onMessage(lws *wsi, uint8_t* buffer, int len) {
    SnpClient *client = clients.at(wsi);
    client->onMessage(buffer, len);
}