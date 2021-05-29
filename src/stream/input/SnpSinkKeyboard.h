#ifndef SNPSERVER_SNPSINKKEYBOARD_H
#define SNPSERVER_SNPSINKKEYBOARD_H

#include "SnpSinkMouse.h"

struct SnpSinkKeyboardOptions : public SnpComponentOptions {
};

class SnpSinkKeyboard : public SnpComponent {
public:
    explicit SnpSinkKeyboard(const SnpSinkKeyboardOptions &options);
    ~SnpSinkKeyboard() override;
private:
    void onInputData(const uint8_t *data, int len, bool complete);
    int fid;
    bool initKeyboard();
    void keyDown(int key);
    void keyUp(int key);
    void destroyKeyboard();
};


#endif //SNPSERVER_SNPSINKKEYBOARD_H
