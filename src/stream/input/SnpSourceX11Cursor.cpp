#include <network/snp.pb.h>
#include "SnpSourceX11Cursor.h"
#include "util/loguru.h"

SnpSourceX11Cursor::SnpSourceX11Cursor(const SnpSourceCursorOptions &options) : SnpComponent(options, "COMPONENT_OUTPUT_CURSOR_X11") {
    this->defaultDisplay = ":0.0";
    this->display = nullptr;
    this->lastCursorSerial = 0;

    addOutputPort(new SnpPort());
}

SnpSourceX11Cursor::~SnpSourceX11Cursor() {
    destroyX11Client();
}

bool SnpSourceX11Cursor::start() {
    SnpComponent::start();
    initX11Client();
    grabberThread = std::thread{[this] () {
        runX11Loop();
    }};
    return true;
}

void SnpSourceX11Cursor::stop() {
    SnpComponent::stop();
    grabberThread.detach();
    destroyX11Client();
}

bool SnpSourceX11Cursor::initX11Client() {
    bool result = true;

    this->display = XOpenDisplay(defaultDisplay.c_str());
    if(display == nullptr) {
        LOG_F(ERROR, "Cannot open display %s.", defaultDisplay.c_str());
        result = false;
        goto error;
    }

    if (!XFixesQueryExtension(display, &xFixesEventBase, &xFixesErrorBase)) {
        LOG_F(ERROR, "XFixesQueryExtension failed, not sending cursor changes");
        result = false;
        goto error;
    }

    rootWindow = DefaultRootWindow(display);

    XFixesSelectCursorInput(display, rootWindow, XFixesDisplayCursorNotifyMask);

    return result;
error:
    return result;
}

void SnpSourceX11Cursor::destroyX11Client() {
    XCloseDisplay(display);
}

[[noreturn]] void SnpSourceX11Cursor::runX11Loop() {
    XEvent ev;
    while(true) {
        XNextEvent(display, &ev);
        if(ev.type == xFixesEventBase + XFixesCursorNotify) {
            XFixesCursorImage *x11Cursor = XFixesGetCursorImage(display);
            if(lastCursorSerial == x11Cursor->cursor_serial) continue;
            lastCursorSerial = x11Cursor->cursor_serial;

            //create a message and send it
            auto streamDataCursor = snp::StreamDataCursor();
            streamDataCursor.set_width(x11Cursor->width);
            streamDataCursor.set_height(x11Cursor->height);
            streamDataCursor.set_hotx(x11Cursor->xhot);
            streamDataCursor.set_hoty(x11Cursor->yhot);
            int sizePixels = x11Cursor->width * x11Cursor->height;
            unsigned long *argb = x11Cursor->pixels;
            auto *rgba = new std::string();
            rgba->resize(sizePixels * 4);
            for(int i = 0; i < sizePixels; i++) {
                (*rgba)[(i*4)]   = (((uint32_t)argb[i])) & 0xff;
                (*rgba)[(i*4)+1] = (((uint32_t)argb[i]) >> 8) & 0xff;
                (*rgba)[(i*4)+2] = (((uint32_t)argb[i]) >> 16) & 0xff;
                (*rgba)[(i*4)+3] = (((uint32_t)argb[i]) >> 24) & 0xff;
            }
            XFree(x11Cursor);
            streamDataCursor.set_allocated_image(rgba);
            std::string data = streamDataCursor.SerializeAsString();
            getOutputPort(0)->onData(getPipeId(), (const uint8_t*)data.c_str(), data.length(), true);
        }
    }
}
