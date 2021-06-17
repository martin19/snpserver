#ifndef SNAPPYVNC_CLIENT_H
#define SNAPPYVNC_CLIENT_H

#include <cstdint>
#include <ctime>
#include <stream/SnpPipe.h>
#include "snappyv1.pb.h"

#include "libwebsockets.h"

typedef std::function<void(const uint8_t *data, int len, bool complete)> StreamListener;

class SnpSocket;

class SnpClient {
public:
    time_t getConnectionStartTs() const;
    SnpClient(SnpSocket *server, struct lws *wsi);

    virtual ~SnpClient();

    bool operator< (const SnpClient &right) const;
    void onMessage(uint8_t *data, int len);
    void send(snappyv1::Message *message);
    void sendStreamData(uint32_t streamId, uint8_t *data, int len);
    void sendStreamChangeAck(uint32_t streamId, SnpPipe* pipe);
    void setStreamListener(uint32_t streamId, StreamListener streamListener);
private:
    std::time_t connectionStartTs;
    struct lws *wsi = nullptr;
    SnpSocket *server = nullptr;

    std::map<uint32_t, SnpPipe*> pipes;
    std::map<uint32_t, StreamListener> streamListeners;

    void onStreamChange(const snappyv1::StreamChange &msg);
    void onStreamChangeInit(const snappyv1::StreamChange &msg);
    void onStreamChangeStart(const snappyv1::StreamChange &msg);
    void onStreamChangeStop(const snappyv1::StreamChange &msg);
    void onStreamChangeDestroy(const snappyv1::StreamChange &msg);

    void onStreamData(const snappyv1::StreamData &msg);
};

#endif //SNAPPYVNC_CLIENT_H
