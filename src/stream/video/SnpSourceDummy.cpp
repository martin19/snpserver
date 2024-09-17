#include "SnpSourceDummy.h"
#include "math.h"
#include <algorithm>
#ifdef _WIN32
#include "windows.h"
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

SnpSourceDummy::SnpSourceDummy(const SnpSourceDummyOptions &options) : SnpComponent(options, "sourceDummy") {
    auto port = new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO);
    auto format = (StreamFormatVideo*)port->getFormat();
    format->width = 1920;
    format->height = 1080;
    addOutputPort(port);

    addProperty(new SnpProperty("fps", PROPERTY_TYPE_DOUBLE));
    addProperty(new SnpProperty("width", PROPERTY_TYPE_UINT32));
    addProperty(new SnpProperty("height", PROPERTY_TYPE_UINT32));

    getProperty("fps")->setValue(options.fps);
    getProperty("width")->setValue(options.width);
    getProperty("height")->setValue(options.height);

    framesRendered = 0;
    frameBuffer = (uint8_t*)calloc(options.width*options.height*4, 1);
}

SnpSourceDummy::~SnpSourceDummy() {
    free(frameBuffer);
}

bool SnpSourceDummy::start() {
    SnpComponent::start();
    LOG_F(INFO, "starting dummy video source");
    renderThread = std::thread(&SnpSourceDummy::renderFrame, this);
    return true;
}

void SnpSourceDummy::stop() {
    SnpComponent::stop();
}

void SnpSourceDummy::setEnabled(bool enabled) {
    SnpComponent::setEnabled(enabled);
}

void SnpSourceDummy::renderFrame() {
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();
    double fps = getProperty("fps")->getValueDouble();

    int8_t dx = 1;
    int8_t dy = 1;
    uint32_t boxWidth2 = 50;
    uint32_t boxHeight2 = 50;
    uint32_t boxPosX = MIN(MAX(floor(rand()*width), width-boxWidth2), boxWidth2);
    uint32_t boxPosY = MIN(MAX(floor(rand()*height), height-boxHeight2), boxHeight2);

    while(this->isRunning()) {
        SnpPort * outputPort = this->getOutputPort(0);

        uint8_t *dst = frameBuffer;
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                *dst = floor(sin(((M_PI/180)*1/(30.0)*y*framesRendered))*255.0); dst++;
                *dst = floor(sin(((M_PI/180)*1/(40.0)*y*framesRendered))*255.0); dst++;
                *dst = floor(sin(((M_PI/180)*1/(50.0)*y*framesRendered))*255.0); dst++;
                *dst = 255; dst++;
            }
        }

        uint32_t xstart = 0;
        uint32_t ystart = 0;
        uint32_t xend = 0;
        uint32_t yend = 0;
        for(int y = ystart; y < yend; y++) {
            for(int x = xstart; x < xend; x++) {
                dst = &frameBuffer[(y*width + x)*4];
                *dst = 255; dst++;
                *dst = 255; dst++;
                *dst = 255; dst++;
            }
        }

        if(boxPosX >= width - boxWidth2) {
            dx = -1;
        }
        if(boxPosX <= boxWidth2) {
            dx = +1;
        }
        if(boxPosY >= height - boxHeight2) {
            dy = -1;
        }
        if(boxPosY <= boxHeight2) {
            dy = +1;
        }
        boxPosX += dx;
        boxPosY += dy;

        outputPort->onData(frameBuffer, width*height*3, true);
        framesRendered++;
        Sleep(1/fps*1000);
    }
}
