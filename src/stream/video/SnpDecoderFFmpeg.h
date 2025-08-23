#ifndef SNPSERVER_SNPDECODERFFMPEG_H
#define SNPSERVER_SNPDECODERFFMPEG_H

//
//#include <stream/SnpComponent.h>
//#include <string>
//#include "codec_api.h"
//#include "stream/video/h264/OpenH264Api.h"
//
//struct SnpDecoderOpenH264Options : public SnpComponentOptions {
//    uint32_t width;
//    uint32_t height;
//    uint32_t qp;
//};
//
//class SnpDecoderOpenH264 : public SnpComponent {
//public:
//    explicit SnpDecoderOpenH264(const SnpDecoderOpenH264Options &options);
//    ~SnpDecoderOpenH264() override;
//
//    bool start() override;
//    void stop() override;
//
//private:
//    OpenH264Api openH264Api;
//
//    void onInputData(uint32_t pipeId, SnpData* data);
//
//    bool openH264DecoderInit();
//    bool openH264DecoderDecode(const uint8_t *srcBuffer, int srcLen);
//    void openH264DecoderDestroy();
//    ISVCDecoder *decoder;
//    uint8_t *yuvBuffer[3];
//    uint8_t *rgbBuffer;
//};


#endif //SNPSERVER_SNPDECODERFFMPEG_H
