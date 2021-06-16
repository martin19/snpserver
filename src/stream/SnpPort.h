#ifndef SNPSERVER_SNPPORT_H
#define SNPSERVER_SNPPORT_H

#include <functional>

class SnpComponent;

enum PortBufferType {
    PORT_TYPE_MMAP = 0,
    PORT_TYPE_COPY = 1,
    PORT_TYPE_BOTH = 2,
};

enum PortStreamType {
    PORT_STREAM_TYPE_GENERIC = 0,
    PORT_STREAM_TYPE_VIDEO = 1,
    PORT_STREAM_TYPE_AUDIO = 2,
};

struct StreamFormat {
};

struct StreamFormatVideo : public StreamFormat {
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerPixel;
};

struct StreamFormatAudio : public StreamFormat {

};


class SnpPort {
public:
    SnpPort();

    SnpPort(PortBufferType bufferType, PortStreamType streamType);

    SnpComponent *getOwner() const;
    void setOwner(SnpComponent *owner);
    virtual ~SnpPort();

    void init();

    SnpPort *targetPort;
    SnpPort *sourcePort;

    static bool connect(SnpPort *sourcePort, SnpPort *targetPort);

    void onData(const uint8_t *data, uint32_t len, bool complete) {
        targetPort->onDataCb(data, len, complete);
    }

    void setOnDataCb(std::function<void(const uint8_t *data, uint32_t len, bool complete)> cb) {
        onDataCb = cb;
    }

    //zero copy buffer info
    std::string device;
    int deviceFd;
    uint8_t *dmaBuf;
    int dmaBufFd;

    //normal buffer info
    uint8_t *buffer;

private:
    PortBufferType bufferType;
    PortStreamType streamType;
    StreamFormat *format;
public:
    [[nodiscard]] StreamFormat *getFormat() const;
private:
    std::function<void(const uint8_t *data, int len, bool complete)> onDataCb = nullptr;
    SnpComponent *owner;
};



#endif //SNPSERVER_SNPPORT_H
