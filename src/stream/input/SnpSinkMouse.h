#ifndef SNPSERVER_SNPSINKMOUSE_H
#define SNPSERVER_SNPSINKMOUSE_H

#include <stream/SnpSink.h>

struct SnpSinkMouseOptions : public SnpSinkOptions {
    int width;
    int height;
};

class SnpSinkMouse : public SnpSink {
public:
    explicit SnpSinkMouse(const SnpSinkMouseOptions &options);
    virtual ~SnpSinkMouse();
    void process(uint8_t *data, int len, bool complete) override;
private:
    int fid;
    int width;
    int height;
    int previousButtonMask;
    bool initMouse();
    bool setMousePosition(int x, int y);
    bool setButton(int32_t buttonMask, int down);
    void destroyMouse();
};


#endif //SNPSERVER_SNPSINKMOUSE_H
