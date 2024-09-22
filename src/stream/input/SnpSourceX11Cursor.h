#ifndef SNPSERVER_SNPSOURCEX11CURSOR_H
#define SNPSERVER_SNPSOURCEX11CURSOR_H

#include <stream/SnpComponent.h>
#include <thread>
#include "X11/Xlib.h"
#include "X11/extensions/Xfixes.h"

struct SnpSourceCursorOptions : public SnpComponentOptions {
};

class SnpSourceX11Cursor : public SnpComponent {
public:
    explicit SnpSourceX11Cursor(const SnpSourceCursorOptions &options);
    ~SnpSourceX11Cursor() override;
    bool start() override;
    void stop() override;
private:

    //x11 client state
    std::string defaultDisplay;
    Display *display;
    Window rootWindow;
    int xFixesEventBase, xFixesErrorBase;
    unsigned long lastCursorSerial;

    std::thread grabberThread;
    bool initX11Client();

    [[noreturn]] void runX11Loop();
    void destroyX11Client();
};


#endif //SNPSERVER_SNPSOURCEX11CURSOR_H
