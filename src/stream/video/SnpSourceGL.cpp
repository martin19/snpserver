#include "util/assert.h"
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <drm_fourcc.h>
#include "SnpSourceGL.h"
#include "util/loguru.h"
#include <util/TimeUtil.h>
#include <math.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

////////////////////// raspberry
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <iostream>
#include <unistd.h>
//#include <GLES2/glext.h>

SnpSourceGL::SnpSourceGL(const SnpSourceGLOptions &options) : SnpComponent(options, "COMPONENT_CAPTURE_GL") {
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO));

    addProperty(new SnpProperty("width", PROPERTY_TYPE_UINT32));
    addProperty(new SnpProperty("height", PROPERTY_TYPE_UINT32));
    addProperty(new SnpProperty("bytesPerPixel", PROPERTY_TYPE_UINT32));

    device = options.device;
    framesCaptured = 0;
    eglDisplay = nullptr;
    eglCtx = nullptr;
    imagePrimary = nullptr;
    imageCapture = nullptr;
}

SnpSourceGL::~SnpSourceGL() {
    grabberThread.detach();
    destroyGL();
    destroyMmap();
    destroyDrm();
}

bool SnpSourceGL::createDumbBo(int deviceFd, uint32_t width, uint32_t height, uint32_t bpp, dumb_bo **pDumbBo) {
    bool result = true;
    drm_mode_create_dumb arg = {0};

    auto dumbBo = new dumb_bo();

    arg.width = width;
    arg.height = height;
    arg.bpp = bpp;

    if(drmIoctl(deviceFd, DRM_IOCTL_MODE_CREATE_DUMB, &arg) != 0) {
        result = false;
        LOG_F(ERROR, "Cannot create dumb bo for capture fb");
        goto error;
    }

    dumbBo->handle = arg.handle;
    dumbBo->size = arg.size;
    dumbBo->pitch = arg.pitch;
    *pDumbBo = dumbBo;

    return result;
    error:
    delete dumbBo;
    return result;
}

bool SnpSourceGL::destroyDumbBo(int deviceFd, dumb_bo *dumbBo) {
    bool result = true;

    drm_mode_destroy_dumb arg = {0};
    arg.handle = dumbBo->handle;

    if(drmIoctl(deviceFd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg) != 0) {
        result = false;
        LOG_F(ERROR, "Cannot create dumb bo for capture fb\n");
        goto error;
    }

    error:
    delete dumbBo;
    return result;
}

bool
SnpSourceGL::createCaptureFb(int deviceFd, uint32_t width, uint32_t height, uint32_t bpp, FramebufferInfo **framebuffer) {
    bool result = true;

    int ret = -1;

    uint32_t pixelFormat = DRM_FORMAT_ARGB8888;
    uint32_t handles[4];
    uint32_t pitches[4];
    uint32_t offsets[4];
    uint64_t modifiers[4];

    memset(handles, 0, sizeof(uint32_t)*4);
    memset(pitches, 0, sizeof(uint32_t)*4);
    memset(offsets, 0, sizeof(uint32_t)*4);
    memset(modifiers, 0, sizeof(uint64_t)*4);

    auto *fb = new FramebufferInfo();
    createDumbBo(deviceFd, width, height, 32, &fb->bo);

    handles[0] = fb->bo->handle;
    pitches[0] = width*4;
    modifiers[0] = DRM_FORMAT_MOD_LINEAR;

    ret = drmModeAddFB2WithModifiers(deviceFd,
                                     width, height,
                                     DRM_FORMAT_XBGR8888,
                                     handles, pitches, offsets, modifiers,
                                     &fb->fbId, 0);

    if (ret < 0) {
        result = false;
        LOG_F(ERROR,"failed to create capture framebuffer: %s", strerror(ret));
        goto error;
    }

    fb->deviceFd = deviceFd;
    fb->fb2Ptr = drmModeGetFB2(deviceFd, fb->fbId);

    if (!fb->fb2Ptr) {
        result = false;
        LOG_F(ERROR, "Cannot open framebuffer %#x", fb->fbId);
        goto error;
    }

    *framebuffer = fb;

    return result;
error:
    return result;
}

bool SnpSourceGL::destroyCaptureFb(FramebufferInfo *framebuffer) {
    bool result = true;

    int ret = -1;

    drmModeFB2Ptr fb2Ptr = drmModeGetFB2(framebuffer->deviceFd, framebuffer->fbId);
    if(fb2Ptr) {
        drmModeFreeFB2(fb2Ptr);
    }

    if(framebuffer->bo) {
        destroyDumbBo(framebuffer->deviceFd, framebuffer->bo);
    }

    delete framebuffer;

    return result;
error:
    return result;
}

bool SnpSourceGL::discoverPrimaryFb(int deviceFd, FramebufferInfo **framebuffer) {
    bool result = true;

    drmModeResPtr resources = drmModeGetResources(deviceFd);
    ASSERT(resources != nullptr);

    //- get first enabled CRTC (primary crtc)
    //- get first primary plane for CRTC
    //- get according framebuffer
    //read whole structure in c++ objects.

    return result;
error:
    return result;
}

bool SnpSourceGL::initDrm() {
    bool result = true;
    StreamFormatVideo* format;

    deviceFd = open(device.c_str(), O_RDWR);
    if (deviceFd < 0) {
        result = false;
        LOG_F(ERROR, "Cannot open modesetting device %s\n", device.c_str());
        return result;
    }

    //get primary framebuffer
    fbPrimary = new FramebufferInfo();
    fbPrimary->deviceFd = deviceFd;

    DrmUtil drmUtil(deviceFd);
    drmUtil.getPrimaryFb(&fbPrimary->fbId);

    fbPrimary->fbPtr = drmModeGetFB(deviceFd, fbPrimary->fbId);
    if (!fbPrimary->fbPtr) {
        result = false;
        LOG_F(ERROR, "Cannot open primary framebuffer %#x", fbPrimary->fbId);
        goto error;
    }

    LOG_F(INFO, "Primary framebuffer found: fb_id=%#x\n",  fbPrimary->fbId);

    //create the capture framebuffer
    if(!createCaptureFb(deviceFd, fbPrimary->fbPtr->width, fbPrimary->fbPtr->height, fbPrimary->fbPtr->bpp, &fbCapture)) {
        result = false;
        LOG_F(ERROR, "Cannot create capture framebuffer.");
        goto error;
    }

    LOG_F(INFO, "Capture framebuffer created: fb_id=%#x\n", fbCapture->fbId);

    drmPrimeHandleToFD(deviceFd, fbPrimary->fbPtr->handle, 0, &fbPrimary->dmaBufFd);
    LOG_F(INFO, "drmPrimeHandleToFD (primary): fd = %d", fbPrimary->dmaBufFd);

    drmPrimeHandleToFD(deviceFd, fbCapture->fb2Ptr->handles[0], 0, &fbCapture->dmaBufFd);
    LOG_F(INFO, "drmPrimeHandleToFD (capture): fd = %d", fbCapture->dmaBufFd);

    //copy zero-copy information to output port
    getOutputPort(0)->device = device;
    getOutputPort(0)->deviceFd = fbCapture->deviceFd;
    getOutputPort(0)->dmaBufFd = fbCapture->dmaBufFd;

    format = (StreamFormatVideo*)getOutputPort(0)->getFormat();
    format->width = fbCapture->fb2Ptr->width;
    format->height = fbCapture->fb2Ptr->height;
    format->bytesPerPixel = 32 / 8;

    this->width = fbCapture->fb2Ptr->width;
    this->height = fbCapture->fb2Ptr->height;
    this->pitch = fbCapture->fb2Ptr->pitches[0];
    this->bytesPerPixel = 32 / 8; //TODO: get rid of this constant

    getProperty("width")->setValue(fbCapture->fb2Ptr->width);
    getProperty("height")->setValue(fbCapture->fb2Ptr->height);
    getProperty("bytesPerPixel")->setValue((uint32_t)(32 / 8));

    return result;
error:
    return result;
}

void SnpSourceGL::destroyDrm() {
    destroyCaptureFb(fbCapture);
    delete fbPrimary;
    delete fbCapture;
    close(deviceFd);
}

bool SnpSourceGL::initMmap() {
    bool result = true;

    mmapFrameBuffer = nullptr;
    mmapFrameBuffer = (uint8_t*)mmap(nullptr, width * height * bytesPerPixel, PROT_READ, MAP_SHARED, fbCapture->dmaBufFd, 0);
    if (mmapFrameBuffer == MAP_FAILED) {
        result = false;
        LOG_F(ERROR, "mmap failed: %s", strerror(errno));
        goto error;
    }

    return result;
error:
    return result;
}

void SnpSourceGL::destroyMmap() {
    if(mmapFrameBuffer) {
        munmap(mmapFrameBuffer, width * height * bytesPerPixel);
    }
}

static const char *fragment =
    "#version 120\n"
    "uniform vec2 res;\n"
    "uniform sampler2D tex;\n"
    "void main() {\n"
    "vec2 uv = gl_FragCoord.xy / res;\n"
    "gl_FragColor = texture2D(tex, uv);\n"
    "}\n";

static const EGLint configAttribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
};

static GLfloat g_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f, //bl
    +1.0f, -1.0f, 0.0f, //tl
    -1.0f, +1.0f, 0.0f, //br
    +1.0f, +1.0f, 0.0f,
    +1.0f, -1.0f, 0.0f,
    -1.0f, +1.0f, 0.0f
};

bool SnpSourceGL::initGL() {
    bool result = true;
    GLuint fragmentShader = -1;

    eglBindAPI(EGL_OPENGL_API);
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOG_F(ERROR, "Failed to get EGL display\n");
        return false;
    }

    EGLint ver_min, ver_maj;
    eglInitialize(eglDisplay, &ver_maj, &ver_min);
    LOG_F(INFO, "EGL: version %d.%d", ver_maj, ver_min);
    LOG_F(INFO, "EGL: EGL_VERSION: '%s'", eglQueryString(eglDisplay, EGL_VERSION));
    LOG_F(INFO, "EGL: EGL_VENDOR: '%s'", eglQueryString(eglDisplay, EGL_VENDOR));
    LOG_F(INFO, "EGL: EGL_CLIENT_APIS: '%s'", eglQueryString(eglDisplay, EGL_CLIENT_APIS));
    LOG_F(INFO, "EGL: EGL_EXTENSIONS: '%s'", eglQueryString(eglDisplay, EGL_EXTENSIONS));

    //get procs for all required extensions
    PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA =
        (PFNEGLEXPORTDMABUFIMAGEMESAPROC)eglGetProcAddress("eglExportDMABUFImageMESA");
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

    EGLAttrib const attribute_list_capture[] = {
        EGL_WIDTH, (int)fbCapture->fb2Ptr->width,
        EGL_HEIGHT, (int)fbCapture->fb2Ptr->height,
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_XRGB8888,
        EGL_DMA_BUF_PLANE0_FD_EXT, fbCapture->dmaBufFd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, (int)fbCapture->fb2Ptr->pitches[0],
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, DRM_FORMAT_MOD_LINEAR & 0xFFFFFFFF,
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, DRM_FORMAT_MOD_LINEAR >> 32,
        EGL_NONE};

    EGLAttrib const attribute_list_primary[] = {
        EGL_WIDTH, (int)fbPrimary->fbPtr->width,
        EGL_HEIGHT, (int)fbPrimary->fbPtr->height,
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_XRGB8888,
        EGL_DMA_BUF_PLANE0_FD_EXT, fbPrimary->dmaBufFd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, (int)fbPrimary->fbPtr->pitch,
//TODO: how to determine correct format!

//        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED & 0xFFFFFFFF,
//        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED >> 32,
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, I915_FORMAT_MOD_X_TILED & 0xFFFFFFFF,
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, I915_FORMAT_MOD_X_TILED >> 32,
        EGL_NONE};

    ASSERT(eglExportDMABUFImageMESA);
    ASSERT(glEGLImageTargetTexture2DOES);

    // Select an appropriate configuration
    EGLConfig eglConfig;
    EGLint num_config;
    eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &num_config);

    // Create a context and make it current
    eglCtx = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, nullptr);
    ASSERT(EGL_NO_CONTEXT != eglCtx);

    ASSERT(eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglCtx));

    // --> egl Setup complete, use context from now on.

    imagePrimary = eglCreateImage(eglDisplay,
                                  EGL_NO_CONTEXT,
                                  EGL_LINUX_DMA_BUF_EXT,
                                  (EGLClientBuffer)nullptr,
                                  attribute_list_primary);

    //create capture image - in linear mode
    imageCapture = eglCreateImage(eglDisplay,
                                  nullptr,
                                  EGL_LINUX_DMA_BUF_EXT,
                                  (EGLClientBuffer)nullptr,
                                  attribute_list_capture);

    ASSERT(imagePrimary);
    ASSERT(imageCapture);

    //compile simple copy shader and create program
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader , 1 , &fragment , nullptr);
    glCompileShader(fragmentShader);
    captureProg = glCreateProgram();          // create program object
    glAttachShader(captureProg, fragmentShader);             // and attach both...
    glLinkProgram(captureProg);

//    //Debug: print compiler log
//    GLchar log[10000];
//    GLint logMaxLength = 10000;
//    GLsizei len;
//    glGetProgramInfoLog(prog, logMaxLength, &len, log);
//    fprintf(stderr, "%s\n", log);

    //generate two textures
    GLuint tex[2];
    glGenTextures(2, tex);

    //bind textures to images
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, imagePrimary);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //THIS IS EFFIN REQUIRED!

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, imageCapture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //THIS IS EFFIN REQUIRED!

    ASSERT(glGetError() == 0);

    glUseProgram(captureProg);
    glVertexAttribPointer(
        0, //vertexPosition_modelspaceID, // The attribute we want to configure
        3,              // size
        GL_FLOAT,            // type
        GL_FALSE,            // normalized?
        0,             // stride
        g_vertex_buffer_data // (void*)0            // array buffer offset
    );
    glEnableVertexAttribArray ( 0 );

    //setup render to texture:
    glActiveTexture(GL_TEXTURE1);
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[1], 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_F(ERROR, "framebuffer incomplete! %d\n", result);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);

    glActiveTexture(GL_TEXTURE0); //is this needed?
//    glBindAttribLocation(stage->shader_program, 0, "vposition");

    printf("uloc: %d\n",glGetUniformLocation(captureProg, "tex"));
    printf("uloc: %d\n",glGetUniformLocation(captureProg, "res"));

    glUseProgram(this->captureProg);
    glUniform1i(glGetUniformLocation(this->captureProg, "tex"), 0); //0 is correct, example from khronos
    glUniform2f(glGetUniformLocation(this->captureProg, "res"), this->width, this->height);
    glViewport(0, 0, this->width, this->height);
    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    return result;
error:
    destroyGL();
    return false;
}

void SnpSourceGL::destroyGL() {
    if(eglDisplay) {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if(imagePrimary) {
            eglDestroyImage(eglDisplay, imagePrimary);
            imagePrimary = nullptr;
        }
        if(imageCapture) {
            eglDestroyImage(eglDisplay, imageCapture);
            imageCapture = nullptr;
        }
        if(eglCtx) {
            eglDestroyContext(eglDisplay, eglCtx);
            eglCtx = nullptr;
        }
        eglTerminate(eglDisplay);
    }
}

void SnpSourceGL::captureFrame() {
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFinish();
    this->framesCaptured++;
}

bool SnpSourceGL::start() {
    SnpComponent::start();
    initDrm();
    initMmap();
    LOG_F(INFO, "starting capturing thread");
    grabberThread = std::thread{[this] () {
        this->initGL();
        //TODO: maybe initGL is required to be executed inside this thread.
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
        while(this->isRunning()) {
            setTimestampStartMs(TimeUtil::getTimeNowMs());
            SnpPort * outputPort = this->getOutputPort(0);
            this->captureFrame();
            setTimestampEndMs(TimeUtil::getTimeNowMs());
            outputPort->onData(getPipeId(), this->mmapFrameBuffer, width*height*bytesPerPixel, true);
            usleep(33333);
        }
#pragma clang diagnostic pop
    }};
    return true;
}

void SnpSourceGL::stop() {
    SnpComponent::stop();
    destroyGL();
    destroyMmap();
    destroyDrm();
    grabberThread.detach();
}
