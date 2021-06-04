#include <iostream>
#include "SnpPort.h"

SnpPort::SnpPort() {
    this->owner = nullptr;
    type = PORT_TYPE_COPY;
}

SnpPort::SnpPort(PortType type) {
    this->owner = nullptr;
    this->type = type;
}

SnpPort::~SnpPort() {
//TODO
}

void SnpPort::init() {
//TODO
}

bool SnpPort::connect(SnpPort *sourcePort, SnpPort *targetPort) {
    sourcePort->targetPort = targetPort;

    if((sourcePort->type == PORT_TYPE_MMAP || sourcePort->type == PORT_TYPE_BOTH) &&
        (targetPort->type == PORT_TYPE_MMAP || targetPort->type == PORT_TYPE_BOTH)) {
        std::cout << sourcePort->device << std::endl;
        targetPort->device = sourcePort->device;
        targetPort->deviceFd = sourcePort->deviceFd;
        targetPort->dmaBuf = sourcePort->dmaBuf;
        targetPort->dmaBufFd = sourcePort->dmaBufFd;
    }

    return true;
}

bool SnpPort::initMmap() {
//    bool result = true;
//    uint8_t *map;
//    map = (uint8_t*)mmap(nullptr, this->width * this->height * this->bpp, PROT_READ, MAP_SHARED, this->dmaBufFd, 0);
//    if (map == MAP_FAILED) {
//        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
//        result = false;
//        goto error;
//    }
//    this->dmaBuf = map;
//    return result;
//    error:
//    return result;
}

bool SnpPort::destroyMmap() {
//    bool result = true;
//    int ret = -1;
//    ret = munmap(this->dmaBuf, this->width * this->height * this->bpp);
//    if (!ret) {
//        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
//        result = false;
//        goto error;
//    }
//    return result;
//    error:
//    return result;
}

SnpComponent *SnpPort::getOwner() const {
    return owner;
}

void SnpPort::setOwner(SnpComponent *owner) {
    SnpPort::owner = owner;
}
