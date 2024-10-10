#ifndef SNAPPYVNC_CLIENT_H
#define SNAPPYVNC_CLIENT_H

#include <cstdint>
#include <ctime>
#include <stream/SnpPipe.h>
#include "snp.pb.h"

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
    void send(snp::Message *message);
    void sendStreamData(uint32_t streamId, uint8_t *data, int len);
    void sendStreamChangeInitOk(uint32_t streamId, SnpPipe* pipe);
    void setStreamListener(uint32_t streamId, StreamListener streamListener);
private:
    std::time_t connectionStartTs;
    struct lws *wsi = nullptr;
    SnpWebsocket *server = nullptr;

    std::map<uint32_t, SnpPipe*> pipes;
    std::map<uint32_t, StreamListener> streamListeners;

    void onStreamChange(const snp::StreamChange &msg);
    void onStreamChangeInit(const snp::StreamChange &msg);
    void onStreamChangeStart(const snp::StreamChange &msg);
    void onStreamChangeStop(const snp::StreamChange &msg);
    void onStreamChangeDestroy(const snp::StreamChange &msg);

    void onStreamData(const snp::StreamData &msg);
};

#endif //SNAPPYVNC_CLIENT_H
