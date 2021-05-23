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

    virtual void startCapture() = 0;
    virtual void stopCapture() = 0;

    virtual bool isFrameReady() = 0;
    virtual void getNextFrame(uint8_t *frame) = 0;
};

#endif //SNPSERVER_SNPSOURCE_H
