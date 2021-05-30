#include "SnpSourceModesetting.h"
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <sys/mman.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

SnpSourceModesetting::SnpSourceModesetting(const SnpSourceModesettingOptions &options): SnpComponent(options) {

    addOutputPort(new SnpPort());
    device = options.device;
    init();

}

SnpSourceModesetting::~SnpSourceModesetting() {
    if(getOutputPort(0)->deviceFd) close(getOutputPort(0)->deviceFd);
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
//    uint32_t fbId = 0xcd;
    uint32_t fbId = 0xcf;

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


    getOutputPort(0)->device = device;
    getOutputPort(0)->deviceFd = deviceFd;
    getOutputPort(0)->dmaBufFd = dmaBufFd;

    this->width = fb->width;
    this->height = fb->height;
    this->pitch = fb->pitch;
    this->bpp = fb->bpp/8; //convert "bits per pixel" to "bytes per pixel"

    return result;
error:
    if(deviceFd) close(deviceFd);
    return result;
}

void SnpSourceModesetting::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
    if(enabled) {
        grabberThread = std::thread{[this] () {
            while(isEnabled()) {
                SnpPort * outputPort = getOutputPort(0);
                outputPort->onData(nullptr, 0, true);
//                if(outputPort && outputPort->targetPort) {
//                    outputPort->onDataCb(nullptr, 0, true);
//                }
                usleep(16666);
            }
        }};
    } else {
        grabberThread.detach();
    }
}

