#ifndef SNPSERVER_SNPENCODERVAH264_H
#define SNPSERVER_SNPENCODERVAH264_H

#include <cstdint>
#include <stream/video/h264/VaBitstream.h>
#include "stream/SnpComponent.h"
#include "va/va.h"

#define SURFACE_NUM 1
#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)>(b)?(a):(b))

struct SnpEncoderVaH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerPixel;
    uint32_t qp;
};

class SnpEncoderVaH264 : public SnpComponent {
public:
    explicit SnpEncoderVaH264(const SnpEncoderVaH264Options &options);
    ~SnpEncoderVaH264() override;

    bool start() override;

    void stop() override;

private:
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    void onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete);

    bool initVaH264Encoder();
    bool encodeFrameVaH264(const uint8_t *framebuffer, uint32_t len);
    void VaH264EncoderDestroy();
//    static void rgba2Yuv(uint8_t *destination, const uint8_t *rgb, int width, int height);

    //taken roughly from h264 encoder example from libva utils
    bool initVa();
    bool setupEncode();

    VAProfile h264Profile;
    int frameBitrate;
    uint32_t initialQp;
    uint32_t minimalQp;
    uint32_t idrPeriod;
    uint32_t iPeriod;
    uint32_t ipPeriod;
    uint32_t frameWidthMbAligned;
    uint32_t frameHeightMbAligned;
    uint32_t encodingFrameNum;
    uint32_t currentFrameNum;
    uint32_t currentFrameType;
    int numShortTerm;

    VADisplay vaDisplay;
    VAProfile vaProfile;

    int constraintSetFlag;
    VAConfigAttrib attrib[VAConfigAttribTypeMax];
    VAConfigAttrib configAttrib[VAConfigAttribTypeMax];
    int encPackedHeaderIdx;

    int configAttribNum;
    VASurfaceID srcSurface[SURFACE_NUM];
    VASurfaceID refSurface[SURFACE_NUM];
    VABufferID codedBuf[SURFACE_NUM];
    VAConfigID configId;
    VAContextID contextId;
    VAEncSequenceParameterBufferH264 seqParam;
    VAEncPictureParameterBufferH264 picParam;
    VAEncSliceParameterBufferH264 sliceParam;
    VAPictureH264 currentCurrPic;
    VAPictureH264 referenceFrames[SURFACE_NUM];
    VAPictureH264 refPicList0_P[SURFACE_NUM];
    VAEntrypoint requestedEntrypoint;
    VAEntrypoint selectedEntrypoint;
    bool h264PackedHeader;

    VaBitstream *bitstream;
    void updateReferenceFrames();
    bool renderSequence();
    bool renderPicture();
    bool renderSlice();
    bool renderPackedSequence();
    bool renderPackedPicture();
    bool renderPackedSlice();

    static void vaInfoCallback(void *context, char *message);
    static void vaErrorCallback(void *context, char *message);
};

#endif //SNPSERVER_SNPENCODERVAH264_H