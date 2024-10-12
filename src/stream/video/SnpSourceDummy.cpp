#include "SnpSourceDummy.h"
#include "math.h"
#include <algorithm>

#ifdef _WIN32
#include "windows.h"
#endif
#include "util/TimeUtil.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

SnpSourceDummy::SnpSourceDummy(const SnpSourceDummyOptions &options) : SnpComponent(options, "COMPONENT_CAPTURE_VIDEO_DUMMY") {
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));

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

void SnpSourceDummy::renderFrame() {
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();
    double fps = getProperty("fps")->getValueDouble();

    initBoxes(200, 200);

    while(this->isRunning()) {
        uint32_t tsBeforePaint = TimeUtil::getTimeNowMs();
        SnpPort * outputPort = this->getOutputPort(0);

        //render background
        uint8_t *dst = frameBuffer;
        for(int y = 0; y < height; y++) {
            int col = 255; //fmod(floor((sin(((M_PI/180)*(1/(100.0))*y))*255.0)+(float)framesRendered), 255.0);
            for(int x = 0; x < width; x++) {
                *dst = col; dst++;
                *dst = col; dst++;
                *dst = col; dst++;
                *dst = 255; dst++;
            }
        }

        renderBoxes();

        outputPort->onData(getPipeId(), frameBuffer, width*height*4, true);
        framesRendered++;

        uint32_t tsAfterPaint = TimeUtil::getTimeNowMs();
        int sleepDurationMs = (int)((1000.0/(double)fps) - (double)(tsAfterPaint-tsBeforePaint));
        if(sleepDurationMs > 0) Sleep(sleepDurationMs);
    }
}

int boxPosX[3];
int boxPosY[3];
int boxDx[3];
int boxDy[3];

void SnpSourceDummy::initBoxes(int boxWidth, int boxHeight) {
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();

    for(int i = 0; i < 3; i++) {
        box[i].dx = 1;
        box[i].dy = 1;
        float r1 = (float)rand()/(float)RAND_MAX;
        float r2 = (float)rand()/(float)RAND_MAX;
        box[i].x = MAX(MIN(floor(r1*width), width-boxWidth/2), boxWidth/2);
        box[i].y = MAX(MIN(floor(r2*height), height-boxHeight/2), boxHeight/2);
        box[i].width = boxWidth;
        box[i].height = boxHeight;
        box[i].r = ((i==0) ? 255 : 128);
        box[i].g = ((i==1) ? 255 : 128);
        box[i].b = ((i==2) ? 255 : 128);
    }
}

void SnpSourceDummy::renderBoxes() {
    int vel = 5;
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();

    uint8_t *dst = frameBuffer;
    for(int i = 0; i < 3; i++) {
        int w2 = box[i].width/2;
        int h2 = box[i].height/2;
        uint32_t xstart = box[i].x - w2;
        uint32_t ystart = box[i].y - h2;
        uint32_t xend = box[i].x + w2;
        uint32_t yend = box[i].y + h2;
        for(uint32_t y = ystart; y < yend; y++) {
            for(uint32_t x = xstart; x < xend; x++) {
                dst = &frameBuffer[(y*width+x)*4];
                *dst = box[i].r; dst++;
                *dst = box[i].g; dst++;
                *dst = box[i].b; dst++;
                *dst = 255; dst++;
            }
        }

        if(box[i].x >= width - w2) {
            box[i].dx = -vel;
        }
        if(box[i].x <= w2) {
            box[i].dx = +vel;
        }
        if(box[i].y >= height - h2) {
            box[i].dy = -vel;
        }
        if(box[i].y <= h2) {
            box[i].dy = +vel;
        }
        box[i].x += box[i].dx;
        box[i].y += box[i].dy;
    }
}