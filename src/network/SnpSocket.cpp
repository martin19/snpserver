#include "SnpSocket.h"

#include "libwebsockets.h"
#include "snappyv1.pb.h"

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
    messageBufferLen = 0;
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
            printf("LWS_CALLBACK_PROTOCOL_INIT\n");
        } break;
        case LWS_CALLBACK_WSI_CREATE: {
            printf("LWS_CALLBACK_WSI_CREATE\n");
        } break;
        case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE: {
            printf("LWS_CALLBACK_HTTP_CONFIRM_UPGRADE\n");
        } break;
        case LWS_CALLBACK_HTTP_BIND_PROTOCOL: {
            printf("LWS_CALLBACK_HTTP_BIND_PROTOCOL\n");
        } break;
        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: {
//            printf("LWS_CALLBACK_EVENT_WAIT_CANCELLED\n");
        } break;
        case LWS_CALLBACK_ADD_HEADERS: {
            printf("LWS_CALLBACK_ADD_HEADERS\n");
        } break;
        case LWS_CALLBACK_ESTABLISHED: {
            printf("LWS_CALLBACK_ESTABLISHED\n");
            if(self != nullptr) {
                auto *client = new SnpClient(self, wsi);
                self->clients.insert(std::pair(wsi, client));
                self->sendServerInfo(wsi);
            }
        } break;
        case LWS_CALLBACK_RECEIVE: {
            printf("LWS_CALLBACK_RECEIVE\n");

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
                lwsl_err("Did not receive a full binary message, packet is fragemented - cannot handle fragmented packets yet.");
            }

            break;

        } break;
        case LWS_CALLBACK_CLOSED: {
            //TODO: remove client pointer and clean up.
            printf("LWS_CALLBACK_CLOSED\n");
        } break;
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
            printf("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION\n");
        } break;
        case LWS_CALLBACK_WSI_DESTROY: {
            printf("LWS_CALLBACK_WSI_DESTROY\n");
        } break;
        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: {
            printf("LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED\n");
        } break;
        case LWS_CALLBACK_CLOSED_HTTP: {
            printf("LWS_CALLBACK_CLOSED_HTTP\n");
        } break;
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: {
            printf("LWS_CALLBACK_FILTER_NETWORK_CONNECTION\n");
        } break;
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: {
            printf("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE\n");
        } break;
        case LWS_CALLBACK_CONNECTING: {
            printf("LWS_CALLBACK_CONNECTING\n");
        } break;
        case LWS_CALLBACK_PROTOCOL_DESTROY: {
            printf("LWS_CALLBACK_PROTOCOL_DESTROY\n");
        } break;
        case LWS_CALLBACK_SERVER_WRITEABLE: {
//            printf("LWS_CALLBACK_SERVER_WRITEABLE\n");
            if(self && self->messageBufferLen) {
                int written = lws_write(wsi, &self->messageBuffer[LWS_PRE], self->messageBufferLen, LWS_WRITE_BINARY);
                printf("%d of %d bytes written\n", written, self->messageBufferLen);
                self->messageBufferLen = 0;
            }
        } break;
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            printf("LWS_CALLBACK_CLIENT_WRITEABLE\n");
        } break;
        case LWS_CALLBACK_HTTP: {
            printf("LWS_CALLBACK_HTTP\n");
        } break;
        default: {
            printf("unhandled callback %d\n", reason);
        }
    }
    return 0;
}

void SnpSocket::sendMessage(snappyv1::Message *msg, struct lws *wsi) {
    //send next message.
    messageBufferLen = msg->ByteSizeLong();
    msg->SerializeToArray(&messageBuffer[LWS_PRE], messageBufferLen);
    lws_callback_on_writable(wsi);
}

void SnpSocket::sendServerInfo(lws* wsi) {
    printf("sendServerInfo\n");

    using namespace snappyv1;
    auto serverInfo = new ServerInfo();
    serverInfo->set_platform(PLATFORM_RASPBERRY);

    //add one source
    Source *source = serverInfo->add_available_sources();
    source->set_type(SOURCE_TYPE_VIDEO);
    source->set_sub_type(SOURCE_SUB_TYPE_X11);

    //parameter width
    {
        Parameter *p = source->add_parameters();
        p->set_type(PARAMETER_TYPE_UINT32);
        p->set_key("width");
        auto *value = new Parameter_ValueUint32();
        value->set_value(1920);
        p->set_allocated_value_uint32(value);
    }

    //parameter height
    {
        Parameter *p = source->add_parameters();
        p->set_type(PARAMETER_TYPE_UINT32);
        p->set_key("height");
        auto *value = new Parameter_ValueUint32();
        value->set_value(1080);
        p->set_allocated_value_uint32(value);
    }

    Encoder *encoder = serverInfo->add_available_encoders();
    encoder->set_type(ENCODER_TYPE_H264_HARDWARE);

    //parameter qp
    {
        Parameter *p = encoder->add_parameters();
        p->set_type(PARAMETER_TYPE_UINT32);
        p->set_key("qp");
        auto *value = new Parameter_ValueUint32();
        value->set_value(42);
        value->set_min(10);
        value->set_max(50);
        p->set_allocated_value_uint32(value);
    }

    //parameter string
    {
        Parameter *p = encoder->add_parameters();
        p->set_type(PARAMETER_TYPE_STRING);
        p->set_key("fookey");
        auto *value = new Parameter_ValueString();
        value->set_value("barvalue");
        p->set_allocated_value_string(value);
    }

    //wrap server info message in envelope
    auto *msgEnvelope = new Message();
    msgEnvelope->set_type(MESSAGE_TYPE_SERVER_INFO);
    msgEnvelope->set_allocated_server_info(serverInfo);

    sendMessage(msgEnvelope, wsi);
}

void SnpSocket::onMessage(lws *wsi, uint8_t* buffer, int len) {
    SnpClient *client = clients.at(wsi);
    client->onMessage(buffer, len);
}