#ifndef SNPSERVER_SNPSINKKEYBOARD_H
#define SNPSERVER_SNPSINKKEYBOARD_H

#include <stream/SnpSink.h>

struct SnpSinkKeyboardOptions : public SnpSinkOptions {
};

class SnpSinkKeyboard : public SnpSink {
public:
    explicit SnpSinkKeyboard(const SnpSinkKeyboardOptions &options);
    ~SnpSinkKeyboard() override;
    void process(uint8_t *data, int len, bool complete) override;
private:
    int fid;
    bool initKeyboard();
    void keyDown(int key);
    void keyUp(int key);
    void destroyKeyboard();
};


#endif //SNPSERVER_SNPSINKKEYBOARD_H
