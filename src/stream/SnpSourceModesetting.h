#ifndef SNPSERVER_SNPSOURCEMODESETTING_H
#define SNPSERVER_SNPSOURCEMODESETTING_H

#include <string>
#include "SnpSource.h"

//typedef enum {
//    videoCaptureCopyTypeMMap = 0,
//    videoCaptureCopyTypeCopy = 1,
//    videoCaptureCopyTypeBoth = 2,
//};

struct SnpSourceModesettingOptions : public SnpSourceOptions {
    std::string device;
};

class SnpSourceModesetting : public SnpSource {
private:
    std::string device;
    int deviceFd;
    int dmaBufFd;
    uint8_t *dmaBuf;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;

    bool init();
    bool initMmap();
    bool destroyMmap();
public:
    explicit SnpSourceModesetting(const SnpSourceModesettingOptions &options);
    ~SnpSourceModesetting() override;

    SnpSourceOutputDescriptor getOutputDescriptor();

    void startCapture() override;
    void stopCapture() override;
    bool isFrameReady() override;
    void getNextFrame(uint8_t *frame) override;
};


#endif //SNPSERVER_SNPSOURCEMODESETTING_H
