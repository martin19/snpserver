#include "SnpSink.h"

void SnpSink::setOnFrameDataCb(std::function<void(uint8_t *, int, bool)> cb) {
    onFrameDataCb = cb;
}
