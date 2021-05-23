#ifndef SNAPPYVNC_SERVER_H
#define SNAPPYVNC_SERVER_H

// The ASIO_STANDALONE define is necessary to use the standalone version of Asio.
// Remove if you are using Boost Asio.
#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <functional>
#include <set>
#include "SnpClient.h"

typedef websocketpp::server<websocketpp::config::asio> server;

class SnpSocket {
public:
    SnpSocket();

    virtual ~SnpSocket();

    void run();
    void send(websocketpp::connection_hdl hdl, uint8_t *buffer, int len);
    void send(websocketpp::connection_hdl hdl, std::string &message);
private:
    std::map<websocketpp::connection_hdl, SnpClient, std::owner_less<websocketpp::connection_hdl>> clients;
public:
    std::map<websocketpp::connection_hdl, SnpClient, std::owner_less<websocketpp::connection_hdl>> &getClients();

private:
    void onOpen(websocketpp::connection_hdl);
    void onClose(websocketpp::connection_hdl);
    void onMessage(websocketpp::connection_hdl, server::message_ptr msg);
    void logConnections();

    server endpoint;
};

#endif //SNAPPYVNC_SERVER_H
