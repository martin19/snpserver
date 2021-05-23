#ifndef SNAPPYVNC_CLIENT_H
#define SNAPPYVNC_CLIENT_H

#include <cstdint>
#include <websocketpp/common/connection_hdl.hpp>
#include <ctime>
#include <stream/SnpPipeline.h>
#include "snappyv1.pb.h"

class SnpSocket;

class SnpClient {
public:
    time_t getConnectionStartTs() const;
    SnpClient(SnpSocket *server, const websocketpp::connection_hdl &hdl);

    virtual ~SnpClient();

    bool operator< (const SnpClient &right) const;
    void onMessage(uint8_t *data, int len);
    void send(uint8_t *data, int len);
    void send(std::string &message);
    void sendStreamData(uint8_t *data, int len);
private:
    std::time_t connectionStartTs;
    SnpSocket *server = nullptr;
    websocketpp::connection_hdl hdl;
    SnpPipeline *fixedVideoPipeline = nullptr;
    void onStreamsChange(const snappyv1::StreamsChange &msg);
    void onStreamData(const snappyv1::StreamData &msg);
};

#endif //SNAPPYVNC_CLIENT_H
