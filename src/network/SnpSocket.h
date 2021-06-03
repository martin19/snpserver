#ifndef SNAPPYVNC_SERVER_H
#define SNAPPYVNC_SERVER_H

#include "libwebsockets.h"
#include <functional>
#include <set>
#include <queue>
#include "SnpClient.h"

class SnpSocket {
public:
    SnpSocket();
    ~SnpSocket();

    void run();
    static int callback_http(struct lws *wsi,
                             lws_callback_reasons reason,
                             void *user, void *in, size_t len);

    void sendMessage(snappyv1::Message *msg, struct lws *wsi);
private:
    struct lws_context_creation_info info;
    struct lws_context *context;
    std::map<lws*, SnpClient*> clients;
    void sendServerInfo(lws* wsi);
    void onMessage(lws *wsi, uint8_t *message, int len);
    uint8_t sendBuffer[1024000];
    int sendBufferLen;

    std::queue<snappyv1::Message*> outputQueue;
};

#endif //SNAPPYVNC_SERVER_H


//    void send(websocketpp::connection_hdl hdl, std::string &message);
//private:
////    std::map<websocketpp::connection_hdl, SnpClient, std::owner_less<websocketpp::connection_hdl>> clients;
//public:
//    std::map<websocketpp::connection_hdl, SnpClient, std::owner_less<websocketpp::connection_hdl>> &getClients();
