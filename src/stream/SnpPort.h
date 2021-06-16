#ifndef SNPSERVER_SNPPORT_H
#define SNPSERVER_SNPPORT_H

#include <functional>

class SnpComponent;

enum PortType {
    PORT_TYPE_MMAP = 0,
    PORT_TYPE_COPY = 1,
    PORT_TYPE_BOTH = 2,
};

struct SnpPortOptions {
    PortType type;
};

class SnpPort {
public:
    SnpPort();

    SnpPort(PortType type);

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

    PortType type;

    //zero copy buffer info
    std::string device;
    int deviceFd;
    uint8_t *dmaBuf;
    int dmaBufFd;

    //normal buffer info
    uint8_t *buffer;

private:
    std::function<void(const uint8_t *data, int len, bool complete)> onDataCb = nullptr;
    SnpComponent *owner;
public:
    SnpComponent *getOwner() const;
    void setOwner(SnpComponent *owner);
};



#endif //SNPSERVER_SNPPORT_H
