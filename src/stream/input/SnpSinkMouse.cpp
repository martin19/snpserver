#include "SnpSinkMouse.h"
#include <linux/uinput.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <network/snappyv1.pb.h>
#include "util/loguru.h"

SnpSinkMouse::SnpSinkMouse(const SnpSinkMouseOptions &options) : SnpComponent(options) {
    fid = -1;
    width = options.width;
    height = options.height;
    previousButtonMask = 0;

    addInputPort(new SnpPort());
    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkMouse::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpSinkMouse::~SnpSinkMouse() {
    destroyMouse();
}

void SnpSinkMouse::setEnabled(bool enabled) {
    if(enabled) {
        initMouse();
    } else {
        destroyMouse();
    }
    SnpComponent::setEnabled(enabled);
}

//TODO: eliminate duplicate
static void emit(int fd, int type, int code, int val) {
    struct input_event ie = {0};

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    size_t size = write(fd, &ie, sizeof(ie));
    if(size < 0) {
        LOG_F(ERROR, "error: write");
    }
}

void SnpSinkMouse::onInputData(const uint8_t *data, int len, bool complete) {
    //TODO: only complete messages are accepted, verify and clean up interface.
    snappyv1::StreamDataPointer streamDataPointer = snappyv1::StreamDataPointer();
    streamDataPointer.ParseFromArray(data, len);

    setMousePosition(streamDataPointer.absx(), streamDataPointer.absy());

    if(streamDataPointer.has_mask()) {
        const int buttonMask = streamDataPointer.mask();

        for(int i = 0; i < 8; i++) {
            int button = 1<<i;
            if((buttonMask & button) && (previousButtonMask & button) == 0) {
                setButton(button, 1);
            } else if ((buttonMask & button) == 0 && (previousButtonMask & button)) {
                setButton(button, 0);
            }
        }
        previousButtonMask = buttonMask;
    }
}

bool SnpSinkMouse::initMouse() {
    fid = -1;
    bool result = true;
    struct uinput_setup usetup = {0};
    struct uinput_abs_setup absSetup = {0};

    fid = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fid == -1) {
        LOG_F(ERROR, "Could not open /dev/uinput. Permissions might be insufficient.");
        result = false;
        goto error;
    }

    /* enable mouse button left, middle, right and wheel events */
    ioctl(fid, UI_SET_EVBIT, EV_KEY);
    ioctl(fid, UI_SET_EVBIT, EV_SYN);
    ioctl(fid, UI_SET_EVBIT, EV_MSC);

    ioctl(fid, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fid, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fid, UI_SET_KEYBIT, BTN_MIDDLE);

    ioctl(fid, UI_SET_EVBIT, EV_ABS);
    ioctl(fid, UI_SET_ABSBIT, ABS_X);
    ioctl(fid, UI_SET_ABSBIT, ABS_Y);

    ioctl(fid, UI_SET_EVBIT, EV_REL);
    ioctl(fid, UI_SET_RELBIT, REL_WHEEL);
    ioctl(fid, UI_SET_RELBIT, REL_HWHEEL);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "Snp mouse");

    ioctl(fid, UI_DEV_SETUP, &usetup);

    memset(&absSetup, 0, sizeof(absSetup));
    absSetup.code = ABS_X;
    absSetup.absinfo.minimum = 0;
    absSetup.absinfo.maximum = width;
    ioctl(fid, UI_ABS_SETUP, &absSetup);
    absSetup.code = ABS_Y;
    absSetup.absinfo.minimum = 0;
    absSetup.absinfo.maximum = height;
    ioctl(fid, UI_ABS_SETUP, &absSetup);

    ioctl(fid, UI_DEV_CREATE);

    /*
     * On UI_DEV_CREATE the kernel will create the device node for this
     * device. We are inserting a pause here so that userspace has time
     * to detect, initialize the new device, and can start listening to
     * the event, otherwise it will not notice the event we are about
     * to send. This pause is only needed in our example code!
     */
//    sleep(1);
error:
    return result;
}

void SnpSinkMouse::setMousePosition(int x, int y) {
//    std::cout << "setmouseposition " << x << "//" << y << std::endl;
    emit(fid, EV_ABS, ABS_X, x);
    emit(fid, EV_ABS, ABS_Y, y);
    emit(fid, EV_SYN, SYN_REPORT, 0);
}

void SnpSinkMouse::setButton(int32_t button, int down) {
    if(button & 1) {
        emit(fid, EV_KEY, BTN_LEFT, down);
    } else if(button & 2) {
        emit(fid, EV_KEY, BTN_MIDDLE, down);
    } else if(button & 4) {
        emit(fid, EV_KEY, BTN_RIGHT, down);
    } else if(button & 8) {
        emit(fid, EV_REL, REL_WHEEL, +1);
    } else if(button & 16) {
        emit(fid, EV_REL, REL_WHEEL, -1);
    }

    emit(fid, EV_SYN, SYN_REPORT, 0);
}

void SnpSinkMouse::destroyMouse() {
//    sleep(1);
    ioctl(fid, UI_DEV_DESTROY);
    close(fid);
}