#ifndef SNPSERVER_SNPENCODER_H
#define SNPSERVER_SNPENCODER_H

#include <cstdint>
#include <string>
#include <functional>
#include "SnpSource.h"

struct SnpEncoderOutputDescriptor {

};

struct SnpEncoderOptions {
    SnpSourceOutputDescriptor inputDescriptor;
    SnpEncoderOutputDescriptor outputDescriptor;
};

//typedef void (*OnFrameDataCb)(uint8_t *data, int len, bool complete);

class SnpEncoder {
public:
    explicit SnpEncoder(const SnpEncoderOptions &options) {};
    virtual ~SnpEncoder() = default;
    virtual void process() = 0;
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

#endif //SNPSERVER_SNPENCODER_H
