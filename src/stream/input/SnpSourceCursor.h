#ifndef SNPSERVER_SNPSOURCECURSOR_H
#define SNPSERVER_SNPSOURCECURSOR_H

#include <stream/SnpComponent.h>
#include <thread>
#include "X11/Xlib.h"
#include "X11/extensions/Xfixes.h"

struct SnpSourceCursorOptions : public SnpComponentOptions {
};

class SnpSourceCursor : public SnpComponent {
public:
    explicit SnpSourceCursor(const SnpSourceCursorOptions &options);
    ~SnpSourceCursor() override;
    void setEnabled(bool enabled) override;
private:

    //x11 client state
    std::string defaultDisplay;
    Display *display;
    Window rootWindow;
    int xFixesEventBase, xFixesErrorBase;
    unsigned long lastCursorSerial;

    std::thread grabberThread;
    bool initX11Client();
    void runX11Loop();
    void destroyX11Client();
};


#endif //SNPSERVER_SNPSOURCECURSOR_H
