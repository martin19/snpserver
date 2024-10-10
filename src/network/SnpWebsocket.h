#ifndef SNAPPYVNC_SERVER_H
#define SNAPPYVNC_SERVER_H

#ifdef HAVE_LIBWEBSOCKETS

#include "libwebsockets.h"
#include <functional>
#include <set>
#include <queue>
#include "SnpClientWebsocket.h"

class SnpWebsocket {
public:
    SnpWebsocket();
    ~SnpWebsocket();

    void run();
    static int callback_http(struct lws *wsi,
                             lws_callback_reasons reason,
                             void *user, void *in, size_t len);

    void sendMessage(snp::Message *msg, struct lws *wsi);
private:
    struct lws_context_creation_info info;
    struct lws_context *context;
    std::map<lws*, SnpClientWebsocket*> clients;
    void sendStreamInfo(lws* wsi);
    void onMessage(lws *wsi, uint8_t *message, int len);
    uint8_t sendBuffer[1024000];
    int sendBufferLen;

    std::queue<snp::Message*> outputQueue;
};

#endif //HAVE_LIBWEBSOCKETS

#endif //SNAPPYVNC_SERVER_H
