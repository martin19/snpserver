#ifndef SNPSERVER_SNPSOURCE_H
#define SNPSERVER_SNPSOURCE_H

#include <cstdint>
#include <string>

enum SourceType {
    SOURCE_TYPE_UNKNOWN = -1,
    SOURCE_TYPE_MMAP = 0,
    SOURCE_TYPE_COPY = 1,
    SOURCE_TYPE_BOTH = 2,
};

struct SnpSourceOptions {
    SourceType sourceType = SOURCE_TYPE_UNKNOWN;
    double fps = -1;
};

struct SnpSourceOutputDescriptor {
    std::string device;
    int deviceFd = -1;
    int dmaBufFd = -1;
    uint8_t *data = nullptr;
    uint32_t width = -1;
    uint32_t height = -1;
    uint32_t bpp = -1;
};

class SnpSource {
public:
    explicit SnpSource(const SnpSourceOptions &options) {};
    virtual ~SnpSource() = default;
    virtual SnpSourceOutputDescriptor getOutputDescriptor() = 0;
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

#endif //SNPSERVER_SNPSOURCE_H
