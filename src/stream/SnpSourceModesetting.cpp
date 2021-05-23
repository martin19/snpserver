#include "SnpSourceModesetting.h"
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <sys/mman.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

SnpSourceModesetting::SnpSourceModesetting(const SnpSourceModesettingOptions &options):SnpSource(options) {
    this->device = options.device;
    init();
}

SnpSourceModesetting::~SnpSourceModesetting() {
    if(deviceFd) close(deviceFd);
}

bool SnpSourceModesetting::init() {
    bool result = true;
    drmModeFBPtr fb;
    int ret = -1;

    uint32_t fbId = 0xce; //with hardware cursor
//  uint32_t fb_id = 0xd1; //cursor plane test

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
//    if(!ret) {
//        result = false;
//        fprintf(stderr, "Cannot get framebuffer file descriptor\n", fbId);
//        goto error;
//    }

    printf("drmPrimeHandleToFD = %d, fd = %d\n", ret, dmaBufFd);

    this->dmaBufFd = dmaBufFd;
    this->width = fb->width;
    this->height = fb->height;
    this->pitch = fb->pitch;
    this->bpp = fb->bpp/8; //convert "bits per pixel" to "bytes per pixel"

    return result;
error:
    if(deviceFd) close(deviceFd);
    return result;
}

bool SnpSourceModesetting::initMmap() {
    bool result = true;
    uint8_t *map;
    map = (uint8_t*)mmap(nullptr, this->width * this->height * this->bpp, PROT_READ, MAP_SHARED, this->dmaBufFd, 0);
    if (map == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        result = false;
        goto error;
    }
    this->dmaBuf = map;
    return result;
error:
    return result;
}

bool SnpSourceModesetting::destroyMmap() {
    bool result = true;
    int ret = -1;
    ret = munmap(this->dmaBuf, this->width * this->height * this->bpp);
    if (!ret) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        result = false;
        goto error;
    }
    return result;
error:
    return result;
}

SnpSourceOutputDescriptor SnpSourceModesetting::getOutputDescriptor() {
    SnpSourceOutputDescriptor descriptor;
    descriptor.width = width;
    descriptor.height = height;
    descriptor.bpp = bpp;
    descriptor.device = device;
    descriptor.deviceFd = deviceFd;
    descriptor.dmaBufFd = dmaBufFd;
    return descriptor;
}

void SnpSourceModesetting::startCapture() {
    //TODO: create capture timer to respect fps
    //TODO: emit frameReady according to fps
}

void SnpSourceModesetting::stopCapture() {
    //TODO: stop capture timer
}

bool SnpSourceModesetting::isFrameReady() {
    //without a timer, there is always a next frame ready.
    return true;
}

void SnpSourceModesetting::getNextFrame(uint8_t *frame) {
    //TODO:
    // * pass pointer to next frame
    // * if memory mapped
}


