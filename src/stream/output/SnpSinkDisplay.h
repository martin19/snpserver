#ifndef SNPSERVER_SNPSINKDISPLAY_H
#define SNPSERVER_SNPSINKDISPLAY_H

#include <stream/SnpComponent.h>

struct SnpSinkDisplayOptions : public SnpComponentOptions {
    uint32_t streamId;
    uint32_t width;
    uint32_t height;
};

class SnpSinkDisplay : public SnpComponent {
public:
    SnpSinkDisplay(const SnpSinkDisplayOptions &options);
    ~SnpSinkDisplay() override;

    bool start() override;
    void stop() override;
    void setEnabled(bool enabled) override;
    void setOnFrameCb(std::function<void()> cb) {
        onFrameCb = cb;
    }
private:
    uint32_t streamId;
    uint32_t width;
    uint32_t height;
    std::function<void()> onFrameCb = nullptr;
};


#endif //SNPSERVER_SNPSINKDISPLAY_H
