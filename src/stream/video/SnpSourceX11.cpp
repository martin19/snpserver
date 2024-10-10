#include <unistd.h>
#include "SnpSourceX11.h"
#include <xcb/xproto.h>
#include <iostream>
#include <X11/Xutil.h>

SnpSourceX11::SnpSourceX11(const SnpSourceX11Options &options) : SnpComponent(options, "sourceX11") {
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO));

    addProperty(new SnpProperty("width", PROPERTY_TYPE_UINT32));
    addProperty(new SnpProperty("height", PROPERTY_TYPE_UINT32));
    addProperty(new SnpProperty("bytesPerPixel", PROPERTY_TYPE_UINT32));

    defaultDisplay = options.display;
    framesCaptured = 0;
}

SnpSourceX11::~SnpSourceX11() {
    grabberThread.detach();
    destroyX11();
}

bool SnpSourceX11::start() {
    SnpComponent::start();
    initX11();
    LOG_F(INFO, "starting capturing thread");
    grabberThread = std::thread{[this] () {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
        while(this->isRunning()) {
            SnpPort * outputPort = this->getOutputPort(0);
//            this->captureFrame();
            XImage *image = XGetImage(X11Display, rootWindow, 0, 0, width, height, AllPlanes, ZPixmap);
            outputPort->onData(getPipeId(), (uint8_t *)image->data, width*height*bytesPerPixel, true);
            XDestroyImage(image);
        }
#pragma clang diagnostic pop
    }};
    return true;
}

void SnpSourceX11::stop() {
    SnpComponent::stop();
    destroyX11();
    grabberThread.detach();
}

bool SnpSourceX11::initX11() {
    bool result = true;
    int snum;
    StreamFormatVideo *format;

    //this->X11Display = XOpenDisplay(defaultDisplay.c_str());
    this->X11Display = XOpenDisplay(":1.0");
    if(X11Display == nullptr) {
        LOG_F(ERROR, "Cannot open display %s.", defaultDisplay.c_str());
        result = false;
        goto error;
    }

    rootWindow = DefaultRootWindow(X11Display);
    snum = DefaultScreen(X11Display);
    width = DisplayWidth(X11Display, snum);
    height = DisplayHeight(X11Display, snum);
    //TODO: move this, use real values determined from display.
    bytesPerPixel = 32 / 8;

    format = (StreamFormatVideo*)getOutputPort(0)->getFormat();
    format->width = width;
    format->height = height;
    format->bytesPerPixel = bytesPerPixel;

    getProperty("width")->setValue((uint32_t)width);
    getProperty("height")->setValue((uint32_t)height);
    getProperty("bytesPerPixel")->setValue((uint32_t)bytesPerPixel);

    return result;
error:
    return result;
}

void SnpSourceX11::destroyX11() {
    //destroy capturePixmap?
    XCloseDisplay(X11Display);
}

//static const char *fragment =
//    "#version 120\n"
//    "uniform vec2 res;\n"
//    "uniform sampler2D tex;\n"
//    "void main() {\n"
//    "vec2 uv = gl_FragCoord.xy / res;\n"
//    "gl_FragColor = texture2D(tex, uv);\n"
//    "}\n";
//
//static const EGLint configAttribs[] = {
//    EGL_BUFFER_SIZE, 32,
//    EGL_RED_SIZE, 8,
//    EGL_GREEN_SIZE, 8,
//    EGL_BLUE_SIZE, 8,
//    EGL_ALPHA_SIZE, 8,
//    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
//    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
//    EGL_NONE
//};
//
//static GLfloat g_vertex_buffer_data[] = {
//    -1.0f, -1.0f, 0.0f, //bl
//    +1.0f, -1.0f, 0.0f, //tl
//    -1.0f, +1.0f, 0.0f, //br
//    +1.0f, +1.0f, 0.0f,
//    +1.0f, -1.0f, 0.0f,
//    -1.0f, +1.0f, 0.0f
//};
//
//struct texture_storage_metadata_t
//{
//    int fourcc;
//    EGLint offset;
//    EGLint stride;
//} texture_storage_metadata;
//
//bool SnpSourceX11::initGL() {
//    StreamFormatVideo* format;
//    bool result = true;
//    GLuint fragmentShader = -1;
//
//    eglBindAPI(EGL_OPENGL_API);
//    eglDisplay = eglGetDisplay(X11Display);
//    if (eglDisplay == EGL_NO_DISPLAY) {
//        LOG_F(ERROR, "Failed to get EGL display\n");
//        return false;
//    }
//
//    EGLint ver_min, ver_maj;
//    eglInitialize(eglDisplay, &ver_maj, &ver_min);
//    LOG_F(INFO, "EGL: version %d.%d", ver_maj, ver_min);
//    LOG_F(INFO, "EGL: EGL_VERSION: '%s'", eglQueryString(eglDisplay, EGL_VERSION));
//    LOG_F(INFO, "EGL: EGL_VENDOR: '%s'", eglQueryString(eglDisplay, EGL_VENDOR));
//    LOG_F(INFO, "EGL: EGL_CLIENT_APIS: '%s'", eglQueryString(eglDisplay, EGL_CLIENT_APIS));
//    LOG_F(INFO, "EGL: EGL_EXTENSIONS: '%s'", eglQueryString(eglDisplay, EGL_EXTENSIONS));
//
//    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES =
//        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
//    PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA =
//        (PFNEGLEXPORTDMABUFIMAGEMESAPROC)eglGetProcAddress("eglExportDMABUFImageMESA");
//
//    ASSERT(glEGLImageTargetTexture2DOES);
//    ASSERT(eglExportDMABUFImageMESA);
//
//    // Select an appropriate configuration
//    EGLConfig eglConfig;
//    EGLint num_config;
//    eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &num_config);
//
//    // Create a context and make it current
//    eglCtx = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, nullptr);
//    ASSERT(EGL_NO_CONTEXT != eglCtx);
//
//    ASSERT(eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglCtx));
//
//    // --> egl Setup complete, use context from now on.
//
//    //compile simple copy shader and create program
//    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader , 1 , &fragment , nullptr);
//    glCompileShader(fragmentShader);
//    captureProg = glCreateProgram();          // create program object
//    glAttachShader(captureProg, fragmentShader);             // and attach both...
//    glLinkProgram(captureProg);
//
//    //generate two textures
//    GLuint tex[2];
//    glGenTextures(2, tex);
//
//    //bind textures to images
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, tex[0]);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //THIS IS EFFIN REQUIRED!
//
//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, tex[1]);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //THIS IS EFFIN REQUIRED!
//
//    ASSERT(glGetError() == 0);
//
//    // create images
//    imagePrimary = eglCreateImage(
//        eglDisplay,
//        eglCtx,
//        EGL_NATIVE_PIXMAP_KHR,
//        (EGLClientBuffer)capturePixmap,
//        nullptr);
//
//    imageCapture = eglCreateImage(
//        eglDisplay,
//        eglCtx,
//        EGL_GL_TEXTURE_2D,
//        (EGLClientBuffer)(uint64_t)tex[1],
//        nullptr);
//
//    eglExportDMABUFImageMESA(eglDisplay,
//                             imagePrimary,
//                             &imageCaptureDmabufFd,
//                             &texture_storage_metadata.stride,
//                             &texture_storage_metadata.offset);
//
//    ASSERT(imagePrimary);
//    ASSERT(imageCapture);
//
//    glUseProgram(captureProg);
//    glVertexAttribPointer(
//        0, //vertexPosition_modelspaceID, // The attribute we want to configure
//        3,              // size
//        GL_FLOAT,            // type
//        GL_FALSE,            // normalized?
//        0,             // stride
//        g_vertex_buffer_data // (void*)0            // array buffer offset
//    );
//    glEnableVertexAttribArray ( 0 );
//
//    //setup render to texture:
//    glActiveTexture(GL_TEXTURE1);
//    glGenFramebuffers(1, &frameBuffer);
//    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[1], 0);
//
//    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//        LOG_F(ERROR, "framebuffer incomplete! %d\n", result);
//    }
//
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
//
//    glActiveTexture(GL_TEXTURE0); //is this needed?
////    glBindAttribLocation(stage->shader_program, 0, "vposition");
//
//    printf("uloc: %d\n",glGetUniformLocation(captureProg, "tex"));
//    printf("uloc: %d\n",glGetUniformLocation(captureProg, "res"));
//
//    glUseProgram(this->captureProg);
//    glUniform1i(glGetUniformLocation(this->captureProg, "tex"), 0); //0 is correct, example from khronos
//    glUniform2f(glGetUniformLocation(this->captureProg, "res"), this->width, this->height);
//    glViewport(0, 0, this->width, this->height);
//    glClearColor(0, 0, 0, 1.0);
//    glClear(GL_COLOR_BUFFER_BIT);
//    glDisable(GL_BLEND);
//
//    return result;
//error:
//    destroyGL();
//    return false;
//}
//
//void SnpSourceX11::destroyGL() {
//    if(eglDisplay) {
//        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//        if(imagePrimary) {
//            eglDestroyImage(eglDisplay, imagePrimary);
//            imagePrimary = nullptr;
//        }
//        if(imageCapture) {
//            eglDestroyImage(eglDisplay, imageCapture);
//            imageCapture = nullptr;
//        }
//        if(eglCtx) {
//            eglDestroyContext(eglDisplay, eglCtx);
//            eglCtx = nullptr;
//        }
//        eglTerminate(eglDisplay);
//    }
//}