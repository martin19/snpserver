#include "SnpEncoder.h"

void SnpEncoder::setOnFrameDataCb(std::function<void(uint8_t *, int, bool)> cb) {
    this->onFrameDataCb = cb;
}
