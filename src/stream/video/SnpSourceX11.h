//
// Created by marti on 25/06/2021.
//

#ifndef SNPSERVER_SNPSOURCEX11_H
#define SNPSERVER_SNPSOURCEX11_H

#include <cstdint>
#include <thread>
#include "X11/Xlib.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "stream/SnpComponent.h"

struct SnpSourceX11Options : public SnpComponentOptions {
    std::string display;
    int fps;
};

class SnpSourceX11 : public SnpComponent {
public:
    explicit SnpSourceX11(const SnpSourceX11Options &options);
    ~SnpSourceX11() override;

    void setEnabled(bool enabled) override;

    bool start() override;

    void stop() override;

    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bytesPerPixel;

    uint32_t framesCaptured;
private:
    bool initX11();
    void destroyX11();

    bool initGL();
    void destroyGL();

    bool initMmap();
    void destroyMmap();


    void captureFrame();

    void onInputData(const uint8_t *data, int len, bool complete);

    std::string defaultDisplay;
    Pixmap capturePixmap;
    GC gc;
    std::thread grabberThread;
    Display *X11Display;
    Window rootWindow;

    EGLDisplay eglDisplay;
    EGLContext eglCtx;
    GLuint captureProg;
    GLuint frameBuffer;
    EGLImage imageCapture;
    EGLImage imagePrimary;

    int imageCaptureDmabufFd;

    //memory mapping
    uint8_t *mmapFrameBuffer;
};


#endif //SNPSERVER_SNPSOURCEX11_H
