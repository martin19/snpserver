#ifndef SNPSERVER_SNPSOURCEGL_H
#define SNPSERVER_SNPSOURCEGL_H

#include <string>
#include <thread>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <util/DrmUtil.h>
#include "stream/SnpComponent.h"

struct dumb_bo {
    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
};

struct FramebufferInfo {
    int deviceFd;
    uint32_t fbId;
    drmModeFBPtr fbPtr;
    drmModeFB2Ptr fb2Ptr;
    dumb_bo *bo;
    int dmaBufFd;
};

struct SnpSourceGLOptions : public SnpComponentOptions {
    std::string device;
    int fps;
};

class SnpSourceGL : public SnpComponent {
public:
    explicit SnpSourceGL(const SnpSourceGLOptions &options);
    ~SnpSourceGL() override;

    bool start() override;
    void stop() override;

    //TODO: how to pass these forward (in a generic way) to encoder?
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bytesPerPixel;

    uint32_t framesCaptured;
private:
    bool initDrm();
    void destroyDrm();

    bool initMmap();
    void destroyMmap();

    bool initGL();
    void destroyGL();

    void captureFrame();

    static bool discoverPrimaryFb(int deviceFd, FramebufferInfo **framebuffer);
    static bool createCaptureFb(int deviceFd, uint32_t width, uint32_t height, uint32_t bpp, FramebufferInfo **framebuffer);
    static bool destroyCaptureFb(FramebufferInfo *framebuffer);
    static bool createDumbBo(int deviceFd, uint32_t width, uint32_t height, uint32_t bpp, dumb_bo** pDumbBo);
    static bool destroyDumbBo(int deviceFd, dumb_bo* dumbBo);
    void onInputData(const uint8_t *data, int len, bool complete);

    std::string device;
    int deviceFd;
    FramebufferInfo *fbPrimary;
    FramebufferInfo *fbCapture;
    std::thread grabberThread;

    //memory mapping
    uint8_t *mmapFrameBuffer;

    EGLDisplay eglDisplay;
    EGLContext eglCtx;
    GLuint captureProg;
    GLuint frameBuffer;
    EGLImage imageCapture;
    EGLImage imagePrimary;
};


#endif //SNPSERVER_SNPSOURCEGL_H
