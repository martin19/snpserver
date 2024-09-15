#ifndef SNAPPYVNC_CLIENT_H
#define SNAPPYVNC_CLIENT_H

#include <cstdint>
#include <ctime>
#include <stream/SnpPipe.h>
#include "snappyv1.pb.h"

#include "libwebsockets.h"

typedef std::function<void(const uint8_t *data, int len, bool complete)> StreamListener;

class SnpWebsocket;

class SnpClientWebsocket {
public:
    time_t getConnectionStartTs() const;
    SnpClientWebsocket(SnpWebsocket *server, struct lws *wsi);

    virtual ~SnpClientWebsocket();

    bool operator< (const SnpClientWebsocket &right) const;
    void onMessage(uint8_t *data, int len);
    void send(snappyv1::Message *message);
    void sendStreamData(uint32_t streamId, uint8_t *data, int len);
    void sendStreamChangeInitOk(uint32_t streamId, SnpPipe* pipe);
    void setStreamListener(uint32_t streamId, StreamListener streamListener);
private:
    std::time_t connectionStartTs;
    struct lws *wsi = nullptr;
    SnpWebsocket *server = nullptr;

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
