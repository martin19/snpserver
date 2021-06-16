#include <iostream>
#include <util/VideoUtil.h>
#include "util/assert.h"
#include "SnpEncoderOpenH264.h"

SnpEncoderOpenH264::SnpEncoderOpenH264(const SnpEncoderOpenH264Options &options) : SnpComponent(options) {
    encoder = nullptr;
    yuvBuffer = nullptr;

    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO));
    addOutputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderOpenH264::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpEncoderOpenH264::~SnpEncoderOpenH264() {
    openH264EncoderDestroy();
}

void SnpEncoderOpenH264::setEnabled(bool enabled) {
    if(enabled) {

        auto *format = (StreamFormatVideo*)getInputPort(0)->sourcePort->getFormat();
        width = format->width;
        height = format->height;
//        frameWidthMbAligned = (width + 15) & (~15);
//        frameHeightMbAligned = (height + 15) & (~15);

        openH264EncoderInit();
    } else {
        openH264EncoderDestroy();
    }
    SnpComponent::setEnabled(enabled);
}

void SnpEncoderOpenH264::onInputData(const uint8_t *data, uint32_t len, bool complete) {
    if(!isEnabled()) return;
    this->openH264EncoderEncode(data, len);
}

bool SnpEncoderOpenH264::openH264EncoderInit() {
    bool result = true;
    int res;

    SEncParamExt paramExt = {};
    paramExt.bEnableFrameSkip = true;
    paramExt.iNumRefFrame = 1;
    paramExt.fMaxFrameRate = 60.0;
    paramExt.iPicWidth = width;
    paramExt.iPicHeight = height;
    paramExt.iTargetBitrate = 20000000;
    paramExt.iMinQp = 25;
    paramExt.iMaxQp = 25;

    res = WelsCreateSVCEncoder (&encoder);
    ASSERT(res == 0);
    ASSERT(encoder != nullptr);

    res = encoder->InitializeExt(&paramExt);
    ASSERT(res == 0);

    yuvBuffer = (uint8_t*)calloc(1, width*height*bpp);

    return result;
error:
    return result;
}

void SnpEncoderOpenH264::openH264EncoderDestroy() {
    if(encoder) {
        encoder->Uninitialize();
        WelsDestroySVCEncoder(encoder);
    }

    free(yuvBuffer);
}

static int frame = 0;

bool SnpEncoderOpenH264::openH264EncoderEncode(const uint8_t *framebuffer, uint32_t len) {
    bool result = true;
    int res;
    uint32_t frameSize;
    SnpPort *outputPort = this->getOutputPort(0);
    SFrameBSInfo info = {};
    SSourcePicture pic = {};

    VideoUtil::rgba2Yuv(yuvBuffer, framebuffer, width, height);

    pic.iPicWidth = width;
    pic.iPicHeight = height;
//  pic.uiTimeStamp
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = pic.iPicWidth;
    pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;
    pic.pData[0] = yuvBuffer;
    pic.pData[1] = pic.pData[0] + width * height;
    pic.pData[2] = pic.pData[1] + (width * height >> 2);

    res = encoder->EncodeFrame(&pic, &info);

    if(res == cmResultSuccess && info.eFrameType != videoFrameTypeSkip) {
        //output bitstream
        int iLayer;
        for (iLayer=0; iLayer < info.iLayerNum; iLayer++)
        {
            SLayerBSInfo* pLayerBsInfo = &info.sLayerInfo[iLayer];

            int iLayerSize = 0;
            int iNalIdx = pLayerBsInfo->iNalCount - 1;
            do {
                iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
                --iNalIdx;
            } while (iNalIdx >= 0);

            outputPort->onData(pLayerBsInfo->pBsBuf, iLayerSize, true);
        }
    }

    return result;
error:
    return result;
}