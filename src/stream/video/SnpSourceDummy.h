//
// Created by marti on 16/09/2024.
//

#ifndef SNPSERVER_SNPSOURCEDUMMY_H
#define SNPSERVER_SNPSOURCEDUMMY_H

#include <cstdint>
#include <thread>
#include "stream/SnpComponent.h"

struct SnpSourceDummyOptions : public SnpComponentOptions {
    double fps;
    uint32_t width;
    uint32_t height;
    uint32_t boxCount;
    uint32_t boxSpeed;
};

struct Box {
    int x;
    int y;
    int width;
    int height;
    int dx;
    int dy;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class SnpSourceDummy : public SnpComponent {
public:
    explicit SnpSourceDummy(const SnpSourceDummyOptions &options);
    ~SnpSourceDummy() override;
    bool start() override;
    void stop() override;
private:
    void renderFrame();

    Box box[1000];
    void initBoxes(int width, int height);
    void renderBoxes();

    uint32_t framesRendered;

    std::thread renderThread;

    //memory mapping
    uint8_t *frameBuffer;
};

#endif //SNPSERVER_SNPSOURCEDUMMY_H
