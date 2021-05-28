#include "SnpSinkKeyboard.h"
#include <linux/uinput.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <memory.h>
#include <network/snappyv1.pb.h>
#include "keymap_atset1_linux.h"

SnpSinkKeyboard::SnpSinkKeyboard(const SnpSinkKeyboardOptions &options) : SnpSink(options) {
    initKeyboard();
}

SnpSinkKeyboard::~SnpSinkKeyboard() {
    destroyKeyboard();
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
        fprintf(stderr, "error: write\n");
    }
}

void SnpSinkKeyboard::process(uint8_t *data, int len, bool complete) {
    snappyv1::StreamDataKeyboard streamDataKeyboard = snappyv1::StreamDataKeyboard();
    streamDataKeyboard.ParseFromArray(data, len);

    bool down = streamDataKeyboard.down();
    uint32_t keysym = streamDataKeyboard.keysym();  //TODO: probably discard this from the protocol completely
    uint32_t keycode = streamDataKeyboard.keycode();

    //https://github.com/rfbproto/rfbproto/blob/master/rfbproto.rst#74121qemu-extended-key-event-message
    //uint32_t xtScancode = keycode < 0x7F ? keycode : 0xe000 | (keycode & 0x7F);

    //printf("scancode=%x, linux code=%d\n",xtScancode, code_map_atset1_to_linux[xtScancode]);
    if(down) {
        keyDown(code_map_atset1_to_linux[keycode]);
    } else {
        keyUp(code_map_atset1_to_linux[keycode]);
    }
}

bool SnpSinkKeyboard::initKeyboard() {
    bool result = true;
    struct uinput_setup usetup = {0};
    struct uinput_abs_setup absSetup = {0};

    fid = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fid == -1) {
        fprintf(stderr, "Could not open /dev/uinput. Permissions might be insufficient.\n");
        result = false;
        goto error;
    }

    /* enable mouse button left and relative events */
    ioctl(fid, UI_SET_EVBIT, EV_KEY);
    for(int key = 0; key < 0x278; key++) {
        ioctl(fid, UI_SET_KEYBIT, key);
    }


    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "Example keyboard");

    ioctl(fid, UI_DEV_SETUP, &usetup);
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

void SnpSinkKeyboard::keyDown(int key) {
    emit(fid, EV_KEY, key, 1);
    emit(fid, EV_SYN, SYN_REPORT, 0);
}

void SnpSinkKeyboard::keyUp(int key) {
    emit(fid, EV_KEY, key, 0);
    emit(fid, EV_SYN, SYN_REPORT, 0);
}

void SnpSinkKeyboard::destroyKeyboard() {
    ioctl(fid, UI_DEV_DESTROY);
    close(fid);
}
