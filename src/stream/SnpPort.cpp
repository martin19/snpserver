#include "SnpPort.h"

SnpPort::SnpPort() {
    //TODO
}

SnpPort::~SnpPort() {
//TODO
}

void SnpPort::init() {
//TODO
}

bool SnpPort::connect(SnpPort *sourcePort, SnpPort *targetPort) {
    sourcePort->targetPort = targetPort;

    targetPort->device = sourcePort->device;
    targetPort->deviceFd = sourcePort->deviceFd;
    targetPort->dmaBuf = sourcePort->dmaBuf;
    targetPort->dmaBufFd = sourcePort->dmaBufFd;
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