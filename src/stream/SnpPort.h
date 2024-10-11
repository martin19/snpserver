#ifndef SNPSERVER_SNPPORT_H
#define SNPSERVER_SNPPORT_H

#include <functional>
#include <map>

class SnpComponent;

enum PortBufferType {
    PORT_TYPE_MMAP = 0,
    PORT_TYPE_COPY = 1,
    PORT_TYPE_BOTH = 2,
};

enum PortStreamType {
    PORT_STREAM_TYPE_GENERIC = 0,
    PORT_STREAM_TYPE_VIDEO_RGB = 1,
    PORT_STREAM_TYPE_VIDEO_RGBA = 2,
    PORT_STREAM_TYPE_VIDEO_H264 = 3,
    PORT_STREAM_TYPE_AUDIO_MP3 = 4,
};

//struct StreamFormat {
//};
//
//struct StreamFormatVideo : public StreamFormat {
//    uint32_t width;
//    uint32_t height;
//    uint32_t bytesPerPixel;
//};
//
//struct StreamFormatAudio : public StreamFormat {
//
//};


class SnpPort {
public:
    SnpPort();

    SnpPort(PortBufferType bufferType, PortStreamType streamType);

    SnpComponent *getOwner() const;
    void setOwner(SnpComponent *owner);
    virtual ~SnpPort();

    void init();
    static bool connect(uint32_t pipeId, SnpPort *sourcePort, SnpPort *targetPort);
    static bool canConnect(SnpPort *sourcePort, SnpPort *targetPort);

    void onData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete);
    void setOnDataCb(std::function<void(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete)> cb);
    PortBufferType getBufferType() const;
    PortStreamType getStreamType() const;

private:
    std::map<uint32_t, SnpPort*> sourcePorts;
    std::map<uint32_t, SnpPort*> targetPorts;
public:
    std::map<uint32_t, SnpPort *> &getSourcePorts();
    std::map<uint32_t, SnpPort *> &getTargetPorts();
    //zero copy buffer info
    std::string device;
    int deviceFd;
    uint8_t *dmaBuf;
    int dmaBufFd;
    //normal buffer info
    uint8_t *buffer;
    PortBufferType bufferType;
    PortStreamType streamType;
    std::function<void(uint32_t pipeId, const uint8_t *data, int len, bool complete)> onDataCb = nullptr;
    SnpComponent *owner;
};



#endif //SNPSERVER_SNPPORT_H
