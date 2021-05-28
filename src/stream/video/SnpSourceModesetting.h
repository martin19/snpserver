#ifndef SNPSERVER_SNPSOURCEMODESETTING_H
#define SNPSERVER_SNPSOURCEMODESETTING_H

#include <string>
#include "../SnpSource.h"

struct SnpSourceModesettingOptions : public SnpSourceOptions {
    std::string device;
};

class SnpSourceModesetting : public SnpSource {
public:
    explicit SnpSourceModesetting(const SnpSourceModesettingOptions &options);
    ~SnpSourceModesetting() override;
private:
    std::string device;
    int deviceFd;
    uint8_t *dmaBuf;
    int dmaBufFd;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    bool init();
    bool initMmap();
    bool destroyMmap();
};


#endif //SNPSERVER_SNPSOURCEMODESETTING_H
