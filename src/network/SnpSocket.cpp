#include "SnpSocket.h"

#include <utility>
#include "SnpProtocol.h"
//#include "util/loguru.h"

SnpSocket::SnpSocket() {
    // Set logging settings
    endpoint.set_error_channels(websocketpp::log::elevel::all);
    endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

    endpoint.set_message_handler(bind(&SnpSocket::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    endpoint.set_open_handler(bind(&SnpSocket::onOpen, this, std::placeholders::_1));
    endpoint.set_close_handler(bind(&SnpSocket::onClose, this, std::placeholders::_1));

    // Initialize Asio
    endpoint.init_asio();
}

SnpSocket::~SnpSocket() {
    printf("Cleaning up socket.\n");
    endpoint.stop_listening();
    endpoint.stop();
}

void SnpSocket::run() {
    // Listen on port 9002
    endpoint.listen(9002);
    printf("Listening on port 9002\n");

    // Queues a connection accept operation
    endpoint.start_accept();

    // Start the Asio io_service run loop
    endpoint.run();
}

void SnpSocket::send(websocketpp::connection_hdl hdl, uint8_t *buffer, int len) {
    endpoint.send(hdl, buffer, len, websocketpp::frame::opcode::binary);
}

void SnpSocket::send(websocketpp::connection_hdl hdl, std::string &message) {
    printf("sending message len=%d\n", message.size());
    endpoint.send(hdl, message, websocketpp::frame::opcode::binary);
}

void SnpSocket::onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    SnpClient client = clients.at(hdl);
    client.onMessage((uint8_t*)msg->get_raw_payload().data(), msg->get_raw_payload().length());
}

void SnpSocket::onClose(websocketpp::connection_hdl hdl) {
    clients.erase(hdl);
    logConnections();
}

void SnpSocket::onOpen(websocketpp::connection_hdl hdl) {
    SnpClient client(this, hdl);
    clients.insert(std::pair(hdl, client));
    SnpProtocol::sendServerInfo(client);
    logConnections();
}

void SnpSocket::logConnections() {
    for(const auto& it : clients) {
        server::connection_ptr c = endpoint.get_con_from_hdl(it.first);
        std::cout << "Host:" << c->get_host() << ", Port:" << c->get_port() << "Ts:" << it.second.getConnectionStartTs() << std::endl;
        std::cout << "Remoteendpoint:" << c->get_remote_endpoint() << std::endl;
    }
}

std::map<websocketpp::connection_hdl, SnpClient, std::owner_less<websocketpp::connection_hdl>> &SnpSocket::getClients() {
    return clients;
}
