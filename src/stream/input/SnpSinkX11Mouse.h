#ifndef SNPSERVER_SNPSINKX11MOUSE_H
#define SNPSERVER_SNPSINKX11MOUSE_H

#include <stream/SnpComponent.h>

struct SnpSinkX11MouseOptions : public SnpComponentOptions {
    int width;
    int height;
};

class SnpSinkX11Mouse : public SnpComponent {
public:
    explicit SnpSinkX11Mouse(const SnpSinkX11MouseOptions &options);
    virtual ~SnpSinkX11Mouse();

    bool start() override;
    void stop() override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    int fid;
    int width;
    int height;
    int previousButtonMask;
    bool initMouse();
    void setMousePosition(int x, int y);
    void setButton(int32_t buttonMask, int down);
    void destroyMouse();
};

#endif //SNPSERVER_SNPSINKX11MOUSE_H
