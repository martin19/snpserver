#ifndef SNPSERVER_SNPSINKMOUSE_H
#define SNPSERVER_SNPSINKMOUSE_H

#include <stream/SnpComponent.h>

struct SnpSinkMouseOptions : public SnpComponentOptions {
    int width;
    int height;
};

class SnpSinkMouse : public SnpComponent {
public:
    explicit SnpSinkMouse(const SnpSinkMouseOptions &options);
    virtual ~SnpSinkMouse();

    void setEnabled(bool enabled) override;

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


#endif //SNPSERVER_SNPSINKMOUSE_H
