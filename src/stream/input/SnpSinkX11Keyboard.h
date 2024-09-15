#ifndef SNPSERVER_SNPSINKX11KEYBOARD_H
#define SNPSERVER_SNPSINKX11KEYBOARD_H

#include "SnpSinkX11Mouse.h"

struct SnpSinkX11KeyboardOptions : public SnpComponentOptions {
};

class SnpSinkX11Keyboard : public SnpComponent {
public:
    explicit SnpSinkX11Keyboard(const SnpSinkX11KeyboardOptions &options);
    ~SnpSinkX11Keyboard() override;

    void setEnabled(bool enabled) override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    int fid;
    bool initKeyboard();
    void keyDown(int key);
    void keyUp(int key);
    void destroyKeyboard();
};


#endif //SNPSERVER_SNPSINKX11KEYBOARD_H
