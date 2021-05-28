#ifndef SNPSERVER_SNPSINK_H
#define SNPSERVER_SNPSINK_H

#include "SnpEncoderPipe.h"

struct SnpSinkOptions {
    SnpEncoderOutputDescriptor inputDescriptor;
};

class SnpSink {
public:
    SnpSink(const SnpSinkOptions &options) {}
    virtual ~SnpSink() = default;
    virtual void process(uint8_t *data, int len, bool complete) = 0;
    void setOnFrameDataCb(std::function<void(uint8_t *data, int len, bool complete)> cb) {
        onFrameDataCb = cb;
    }
    bool isEnabled() const {
        return enabled;
    }
    void setEnabled(bool enabled) {
        this->enabled = enabled;
    }
protected:
    std::function<void(uint8_t *data, int len, bool complete)> onFrameDataCb = nullptr;
private:
    bool enabled;
};

#endif //SNPSERVER_SNPSINK_H