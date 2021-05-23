#ifndef SNPSERVER_SNPSINK_H
#define SNPSERVER_SNPSINK_H

#include "SnpPipeline.h"

struct SnpSinkOptions {
    SnpEncoderOutputDescriptor inputDescriptor;
};

class SnpSink {
public:
    SnpSink(const SnpSinkOptions &options) {}
    virtual ~SnpSink() = default;
    virtual void process(uint8_t *data, int len, bool complete) = 0;
    void setOnFrameDataCb(std::function<void(uint8_t *data, int len, bool complete)> cb);
protected:
    std::function<void(uint8_t *data, int len, bool complete)> onFrameDataCb = nullptr;
};

#endif //SNPSERVER_SNPSINK_H