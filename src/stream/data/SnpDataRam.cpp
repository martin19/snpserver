#include "SnpDataRam.h"

SnpDataRam::SnpDataRam(uint8_t *data, uint32_t len, bool complete) : data(data), len(len), complete(complete) {
}

uint8_t *SnpDataRam::getData() {
    return data;
}

uint32_t SnpDataRam::getLen() {
    return len;
}

bool SnpDataRam::getComplete() {
    return complete;
}
