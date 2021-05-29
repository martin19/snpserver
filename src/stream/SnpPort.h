#ifndef SNPSERVER_SNPPORT_H
#define SNPSERVER_SNPPORT_H

#include <functional>

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
    virtual ~SnpPort();

    void init();
    static bool connect(SnpPort *portFrom, SnpPort *portTo);

    std::function<void(const uint8_t *data, int len, bool complete)> onDataCb = nullptr;

    PortType type;

    //zero copy buffer info
    std::string device;
    int deviceFd;
    uint8_t *dmaBuf;
    int dmaBufFd;

    //normal buffer info
    uint8_t *buffer;

    void setOnDataCb(std::function<void(const uint8_t *data, int len, bool complete)> cb) {
        onDataCb = cb;
    }

private:
    bool initMmap();
    bool destroyMmap();
};



#endif //SNPSERVER_SNPPORT_H
