#ifndef SNPSERVER_SNPSOURCEMODESETTING_H
#define SNPSERVER_SNPSOURCEMODESETTING_H

#include <string>
#include "../SnpSource.h"

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
};


#endif //SNPSERVER_SNPSOURCEMODESETTING_H
