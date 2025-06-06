#include "SnpSourceDummy.h"
#include "math.h"

#ifdef _WIN32
#include "windows.h"
#endif
#include "util/TimeUtil.h"
#include "stream/data/SnpDataRam.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

SnpSourceDummy::SnpSourceDummy(const SnpSourceDummyOptions &options) : SnpComponent(options, "COMPONENT_CAPTURE_VIDEO_DUMMY") {
    addOutputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    addProperty(new SnpProperty("boxCount", options.boxCount));
    addProperty(new SnpProperty("boxSpeed", options.boxSpeed));

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

        SnpDataRam ram(frameBuffer, width*height*4, true);
        outputPort->onData(getPipeId(), &ram);
        framesRendered++;

        uint32_t tsAfterPaint = TimeUtil::getTimeNowMs();
        int sleepDurationMs = (int)((1000.0/(double)fps) - (double)(tsAfterPaint-tsBeforePaint));
        if(sleepDurationMs > 0) Sleep(sleepDurationMs);
    }
}

void SnpSourceDummy::initBoxes(int boxWidth, int boxHeight) {
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();
    uint32_t boxCount = getProperty("boxCount")->getValueUint32();

    for(int i = 0; i < boxCount; i++) {
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
    uint32_t width = getProperty("width")->getValueUint32();
    uint32_t height = getProperty("height")->getValueUint32();
    uint32_t boxCount = getProperty("boxCount")->getValueUint32();
    uint32_t boxSpeed = getProperty("boxSpeed")->getValueUint32();

    uint8_t *dst = frameBuffer;
    for(int i = 0; i < boxCount; i++) {
        int w2 = box[i].width/2;
        int h2 = box[i].height/2;
        uint32_t xstart = MAX(0, box[i].x - w2);
        uint32_t ystart = MAX(0, box[i].y - h2);
        uint32_t xend = MIN(width, box[i].x + w2);
        uint32_t yend = MIN(height, box[i].y + h2);
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
            box[i].dx = -boxSpeed;
        }
        if(box[i].x <= w2) {
            box[i].dx = +boxSpeed;
        }
        if(box[i].y >= height - h2) {
            box[i].dy = -boxSpeed;
        }
        if(box[i].y <= h2) {
            box[i].dy = +boxSpeed;
        }
        box[i].x += box[i].dx;
        box[i].y += box[i].dy;
    }
}