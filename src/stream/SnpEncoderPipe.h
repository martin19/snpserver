#ifndef SNPSERVER_SNPENCODERPIPE_H
#define SNPSERVER_SNPENCODERPIPE_H

#include "SnpSource.h"
#include "SnpEncoder.h"
#include "SnpSink.h"

struct SnpPipelineOptions {
    class SnpSource *source;
    class SnpEncoder *encoder;
    class SnpSink *sink;
};

class SnpEncoderPipe {
private:
    SnpPipelineOptions options {};
    SnpSource *source;
    SnpEncoder *encoder;
    SnpSink *sink;
    bool running = false;
public:
    SnpEncoderPipe(SnpPipelineOptions &options);
    SnpSource *getSource();
    SnpEncoder *getEncoder();
    SnpSink *getSink();

    void start();
    void stop();

    void onFrameDataCb(uint8_t *buffer, int len, bool complete);
};


#endif //SNPSERVER_SNPENCODERPIPE_H
