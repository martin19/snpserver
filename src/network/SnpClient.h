#ifndef SNAPPYVNC_CLIENT_H
#define SNAPPYVNC_CLIENT_H

#include <cstdint>
#include <ctime>
#include <stream/SnpEncoderPipe.h>
#include "snappyv1.pb.h"

#include "libwebsockets.h"

class SnpSocket;

class SnpClient {
public:
    time_t getConnectionStartTs() const;
    SnpClient(SnpSocket *server, struct lws *wsi);

    virtual ~SnpClient();

    bool operator< (const SnpClient &right) const;
    void onMessage(uint8_t *data, int len);
    void send(snappyv1::Message *message);
    void sendStreamData(uint8_t *data, int len);
private:
    std::time_t connectionStartTs;
    struct lws *wsi = nullptr;
    SnpSocket *server = nullptr;
    SnpEncoderPipe *fixedVideoPipeline = nullptr;
    void onStreamsChange(const snappyv1::StreamsChange &msg);
    void onStreamData(const snappyv1::StreamData &msg);
};

#endif //SNAPPYVNC_CLIENT_H
