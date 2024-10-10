#include <iostream>
#include "SnpPort.h"

SnpPort::SnpPort() {
    owner = nullptr;
    sourcePort = nullptr;
    targetPort = nullptr;
    bufferType = PORT_TYPE_COPY;
    streamType = PORT_STREAM_TYPE_GENERIC;
}

SnpPort::SnpPort(PortBufferType bufferType, PortStreamType streamType) {
    owner = nullptr;
    sourcePort = nullptr;
    targetPort = nullptr;
    this->bufferType = bufferType;
    this->streamType = streamType;
}

SnpPort::~SnpPort() {
}

void SnpPort::init() {
//TODO
}

bool SnpPort::connect(SnpPort *sourcePort, SnpPort *targetPort) {
    sourcePort->targetPort = targetPort;
    targetPort->sourcePort = sourcePort;

//    if((sourcePort->type == PORT_TYPE_MMAP || sourcePort->type == PORT_TYPE_BOTH) &&
//        (targetPort->type == PORT_TYPE_MMAP || targetPort->type == PORT_TYPE_BOTH)) {
//        std::cout << sourcePort->device << std::endl;
//        targetPort->device = sourcePort->device;
//        targetPort->deviceFd = sourcePort->deviceFd;
//        targetPort->dmaBuf = sourcePort->dmaBuf;
//        targetPort->dmaBufFd = sourcePort->dmaBufFd;
//    }

    return true;
}

SnpComponent *SnpPort::getOwner() const {
    return owner;
}

void SnpPort::setOwner(SnpComponent *owner) {
    SnpPort::owner = owner;
}

PortBufferType SnpPort::getBufferType() const {
    return bufferType;
}

PortStreamType SnpPort::getStreamType() const {
    return streamType;
}
