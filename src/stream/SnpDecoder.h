#ifndef SNPSERVER_SNPDECODER_H
#define SNPSERVER_SNPDECODER_H

#include <cstdint>
#include <string>
#include <functional>
#include "SnpSource.h"

struct SnpDecoderOutputDescriptor {

};

struct SnpDecoderOptions {
    SnpSourceOutputDescriptor inputDescriptor;
    SnpDecoderOutputDescriptor outputDescriptor;
};

class SnpDecoder {
public:
    explicit SnpDecoder(const SnpDecoderOptions &options) {};
    virtual ~SnpDecoder() = default;

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


#endif //SNPSERVER_SNPDECODER_H
