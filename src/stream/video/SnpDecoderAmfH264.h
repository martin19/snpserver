#ifndef SNPSERVER_SNPDECODERAMFH264_H
#define SNPSERVER_SNPDECODERAMFH264_H

#include "stream/SnpComponent.h"
#include "public/include/core/Context.h"
#include "public/common/DataStream.h"
#include "public/include/components/Component.h"

struct SnpDecoderAmfH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
};

class SnpDecoderAmfH264 : public SnpComponent {
public:
    SnpDecoderAmfH264(const SnpDecoderAmfH264Options &options);
    ~SnpDecoderAmfH264() override;

    bool start() override;
    void stop() override;
private:
    void onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete);

    bool decoderInit();
    void decoderDestroy();

    uint32_t width;
    uint32_t height;
    amf::AMFContextPtr      context;
    amf::AMFComponentPtr    decoder;
//  amf::AMFDataStreamPtr   datastream;
//  BitStreamParserPtr      parser;
    amf::AMFBufferPtr amfInputBuffer;
//    uint8_t *yuvBuffer[3];
    uint8_t *rgbaBuffer;
};


#endif //SNPSERVER_SNPDECODERAMFH264_H
