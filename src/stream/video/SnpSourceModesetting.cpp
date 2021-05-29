#include "SnpSourceModesetting.h"
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <sys/mman.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

SnpSourceModesetting::SnpSourceModesetting(const SnpSourceModesettingOptions &options): SnpComponent(options) {

    addOutput(new SnpPort());
    device = options.device;
    init();

}

SnpSourceModesetting::~SnpSourceModesetting() {
    if(getOutput(0)->deviceFd) close(getOutput(0)->deviceFd);
}

bool SnpSourceModesetting::init() {
    bool result = true;
    drmModeFBPtr fb;
    int ret = -1;

    int deviceFd = -1;
    int dmaBufFd = -1;

//    uint32_t fbId = 0xce; //with hardware cursor
//    uint32_t fbId = 0xcf; //with hardware cursor
//  uint32_t fb_id = 0xd1; //cursor plane test

//    uint32_t fbId = 0xcd; //with hardware cursor
    uint32_t fbId = 0xcd;
//    uint32_t fbId = 0xcf;

    //TODO: determine primary framebufferId!
    //TODO: how to detect framebuffer has changed?

    deviceFd = open(device.c_str(), O_RDONLY);
    if (deviceFd < 0) {
        result = false;
        fprintf(stderr, "Cannot open modesetting device %s\n", device.c_str());
        goto error;
    }

    fb = drmModeGetFB(deviceFd, fbId);
    if (!fb) {
        result = false;
        fprintf(stderr, "Cannot open framebuffer %#x\n", fbId);
        goto error;
    }

    printf("fb_id=%#x width=%u height=%u pitch=%u bpp=%u depth=%u handle=%#x\n",
           fbId, fb->width, fb->height, fb->pitch, fb->bpp, fb->depth, fb->handle);

    ret = drmPrimeHandleToFD(deviceFd, fb->handle, 0, &dmaBufFd);

    printf("drmPrimeHandleToFD = %d, fd = %d\n", ret, dmaBufFd);


    getOutput(0)->device = device;
    getOutput(0)->deviceFd = deviceFd;
    getOutput(0)->dmaBufFd = dmaBufFd;

    this->width = fb->width;
    this->height = fb->height;
    this->pitch = fb->pitch;
    this->bpp = fb->bpp/8; //convert "bits per pixel" to "bytes per pixel"

    return result;
error:
    if(deviceFd) close(deviceFd);
    return result;
}

//void SnpSourceModesetting::startCapture() {
//    //TODO: create capture timer to respect fps
//    //TODO: emit frameReady according to fps
//}
//
//void SnpSourceModesetting::stopCapture() {
//    //TODO: stop capture timer
//}
//
//bool SnpSourceModesetting::isFrameReady() {
//    //without a timer, there is always a next frame ready.
//    getOutput(0)->onDataCb(nullptr, 0, true);
//    return true;
//}
//
//void SnpSourceModesetting::getNextFrame(uint8_t *frame) {
//    //TODO:
//    // * pass pointer to next frame
//    // * if memory mapped
//}


