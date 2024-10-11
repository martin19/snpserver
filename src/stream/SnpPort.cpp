#include <iostream>
#include "SnpPort.h"

SnpPort::SnpPort() {
    owner = nullptr;
    bufferType = PORT_TYPE_COPY;
    streamType = PORT_STREAM_TYPE_GENERIC;
}

SnpPort::SnpPort(PortBufferType bufferType, PortStreamType streamType) {
    owner = nullptr;
    this->bufferType = bufferType;
    this->streamType = streamType;
}

SnpPort::~SnpPort() {
}

void SnpPort::init() {
//TODO
}

bool SnpPort::canConnect(SnpPort *sourcePort, SnpPort *targetPort) {
    if(sourcePort->getStreamType() == PORT_STREAM_TYPE_GENERIC ||
        targetPort->getStreamType() == PORT_STREAM_TYPE_GENERIC) return true;
    if(sourcePort->getStreamType() == targetPort->getStreamType()) {
        return true;
    }
    return false;
}

bool SnpPort::connect(uint32_t pipeId, SnpPort *sourcePort, SnpPort *targetPort) {
    sourcePort->getTargetPorts().insert(std::pair(pipeId, targetPort));
    targetPort->getSourcePorts().insert(std::pair(pipeId, sourcePort));
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

void SnpPort::onData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    SnpPort* targetPort = getTargetPorts().at(pipeId);
    if(targetPort == nullptr) return;
    targetPort->onDataCb(pipeId, data, len, complete);
}

void SnpPort::setOnDataCb(std::function<void(uint32_t, const uint8_t *, uint32_t, bool)> cb) {
    onDataCb = cb;
}

std::map<uint32_t, SnpPort *> &SnpPort::getSourcePorts() {
    return sourcePorts;
}

std::map<uint32_t, SnpPort *> &SnpPort::getTargetPorts() {
    return targetPorts;
}

