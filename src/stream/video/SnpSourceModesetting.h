#ifndef SNPSERVER_SNPSOURCEMODESETTING_H
#define SNPSERVER_SNPSOURCEMODESETTING_H

#include <string>
#include <thread>
#include "../SnpComponent.h"

struct SnpSourceModesettingOptions : public SnpComponentOptions {
    std::string device;
    int fps;
};

class SnpSourceModesetting : public SnpComponent {
public:
    explicit SnpSourceModesetting(const SnpSourceModesettingOptions &options);
    ~SnpSourceModesetting() override;

    void setEnabled(bool enabled) override;

    //TODO: how to pass these forward (in a generic way) to encoder?
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
private:
    bool init();
    std::string device;
    std::thread grabberThread;
};

#endif //SNPSERVER_SNPSOURCEMODESETTING_H
