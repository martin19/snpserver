#ifndef SNPSERVER_SNPPIPELINE_H
#define SNPSERVER_SNPPIPELINE_H

#include "SnpSource.h"
#include "SnpEncoder.h"
#include "SnpSink.h"

struct SnpPipelineOptions {
    class SnpSource *source;
    class SnpEncoder *encoder;
    class SnpSink *sink;
};

class SnpPipeline {
private:
    SnpPipelineOptions options {};
    SnpSource *source;
    SnpEncoder *encoder;
    SnpSink *sink;
    bool running = false;
public:
    SnpPipeline(SnpPipelineOptions &options);
    SnpSource *getSource();
    SnpEncoder *getEncoder();
    SnpSink *getSink();

    void start();
    void stop();

    void onFrameDataCb(uint8_t *buffer, int len, bool complete);
};


#endif //SNPSERVER_SNPPIPELINE_H
