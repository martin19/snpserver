#include <network/snappyv1.pb.h>
#include "SnpSourceCursor.h"

SnpSourceCursor::SnpSourceCursor(const SnpSourceCursorOptions &options) : SnpComponent(options) {
    this->defaultDisplay = ":0.0";
    this->display = nullptr;
    this->lastCursorSerial = 0;

    this->addOutputPort(new SnpPort());

    this->initX11Client();
}

SnpSourceCursor::~SnpSourceCursor() {
    this->destroyX11Client();
}

bool SnpSourceCursor::initX11Client() {
    bool result = true;

    this->display = XOpenDisplay(defaultDisplay.c_str());
    if(display == nullptr) {
        fprintf(stderr, "Cannot open display %s.\n", defaultDisplay.c_str());
        result = false;
        goto error;
    }

    if (!XFixesQueryExtension(display, &xFixesEventBase, &xFixesErrorBase)) {
        fprintf(stderr, "XFixesQueryExtension failed, not sending cursor changes\n");
        result = false;
        goto error;
    }

    rootWindow = DefaultRootWindow(display);

    XFixesSelectCursorInput(display, rootWindow, XFixesDisplayCursorNotifyMask);



    return result;
error:
    return result;
}

void SnpSourceCursor::destroyX11Client() {

}

void SnpSourceCursor::runX11Loop() {
    XEvent ev;
    while(this->isEnabled()) {
        XNextEvent(display, &ev);
        if(ev.type == xFixesEventBase + XFixesCursorNotify) {
            XFixesCursorImage *x11Cursor = XFixesGetCursorImage(display);
            if(lastCursorSerial == x11Cursor->cursor_serial) continue;
            lastCursorSerial = x11Cursor->cursor_serial;

            //create a message and send it
            auto streamDataCursor = snappyv1::StreamDataCursor();
            streamDataCursor.set_width(x11Cursor->width);
            streamDataCursor.set_height(x11Cursor->height);
            streamDataCursor.set_hotx(x11Cursor->xhot);
            streamDataCursor.set_hoty(x11Cursor->yhot);
            auto *image = new std::string((uint8_t *)x11Cursor->pixels,
                                                 ((uint8_t *)x11Cursor->pixels) +
                                                 x11Cursor->width * x11Cursor->height * 4);
            streamDataCursor.set_allocated_image(image);
            std::string data = streamDataCursor.SerializeAsString();
            getOutputPort(0)->onData((const uint8_t*)data.c_str(), data.length(), true);
        }
    }
}

void SnpSourceCursor::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
    if(enabled) {
        grabberThread = std::thread{[this] () {
            runX11Loop();
        }};
    } else {
        grabberThread.detach();
    }
}

