#ifndef SNPSERVER_SNPSINKDISPLAY_H
#define SNPSERVER_SNPSINKDISPLAY_H

#include <stream/SnpComponent.h>
#include <QImage>

struct SnpSinkDisplayOptions : public SnpComponentOptions {
    uint32_t streamId;
    uint32_t width;
    uint32_t height;
};

class SnpSinkDisplay : public SnpComponent {
public:
    SnpSinkDisplay(const SnpSinkDisplayOptions &options);
    ~SnpSinkDisplay() override;

    void onInputData(const uint8_t *data, int len, bool complete);
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
    QImage *qImage;

    std::function<void()> onFrameCb = nullptr;
public:
    QImage *getQImage() const;
};


#endif //SNPSERVER_SNPSINKDISPLAY_H
