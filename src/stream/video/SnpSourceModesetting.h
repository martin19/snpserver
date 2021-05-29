#ifndef SNPSERVER_SNPSOURCEMODESETTING_H
#define SNPSERVER_SNPSOURCEMODESETTING_H

#include <string>
#include "../SnpComponent.h"

struct SnpSourceModesettingOptions : public SnpComponentOptions {
    std::string device;
    int fps;
};

class SnpSourceModesetting : public SnpComponent {
public:
    explicit SnpSourceModesetting(const SnpSourceModesettingOptions &options);
    ~SnpSourceModesetting() override;
private:
    bool init();
    std::string device;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
};

#endif //SNPSERVER_SNPSOURCEMODESETTING_H
