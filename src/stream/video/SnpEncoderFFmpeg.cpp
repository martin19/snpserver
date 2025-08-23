#include <iostream>
#include <util/VideoUtil.h>
#include "util/assert.h"
#include "stream/data/SnpDataRam.h"
#include "SnpEncoderFFmpeg.h"

SnpEncoderFFmpeg::SnpEncoderFFmpeg(const SnpEncoderFFmpegOptions &options) : SnpComponent(options, "COMPONENT_ENCODER_FFMPEG") {

    addInputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    addOutputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_H264));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("qp", options.qp));

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderFFmpeg::onInputData, this, std::placeholders::_1, std::placeholders::_2));
}

SnpEncoderFFmpeg::~SnpEncoderFFmpeg() {
//    openH264EncoderDestroy();
}

bool SnpEncoderFFmpeg::start() {
    SnpComponent::start();

    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();
//        frameWidthMbAligned = (width + 15) & (~15);
//        frameHeightMbAligned = (height + 15) & (~15);

//    openH264EncoderInit();
    return true;
}

void SnpEncoderFFmpeg::stop() {
    SnpComponent::stop();
//    openH264EncoderDestroy();
}

void SnpEncoderFFmpeg::onInputData(uint32_t pipeId, SnpData* data) {
    if(!isRunning()) return;
//    if(auto* ram = dynamic_cast<SnpDataRam*>(data)) {
//        this->openH264EncoderEncode(ram->getData(), ram->getLen());
//    }
}
//
//bool SnpEncoderFFmpeg::openH264EncoderInit() {
//    bool result = true;
//
//    double fps = getProperty("fps")->getValueDouble();
//    int res;
//
//    SEncParamExt paramExt = {};
//    paramExt.bEnableFrameSkip = true;
//    paramExt.uiIntraPeriod = 2;
//    paramExt.iNumRefFrame = 1;
//    paramExt.fMaxFrameRate = (float)fps;
//    paramExt.iPicWidth = (int)width;
//    paramExt.iPicHeight = (int)height;
//    paramExt.iTargetBitrate = 30000000;
//    paramExt.iMinQp = 5;
//    paramExt.iMaxQp = 10;
//    paramExt.iUsageType = SCREEN_CONTENT_REAL_TIME;
//
//    res = openH264Api.welsCreateSVCEncoderFunc (&encoder);
//    ASSERT(res == 0);
//    ASSERT(encoder != nullptr);
//
//    res = encoder->InitializeExt(&paramExt);
//    ASSERT(res == 0);
//
//    yuvBuffer = (uint8_t*)calloc(1, width*height*3/2);
//
//    return result;
//    error:
//    return result;
//}
//
//void SnpEncoderFFmpeg::openH264EncoderDestroy() {
//    if(encoder) {
//        encoder->Uninitialize();
//        openH264Api.welsDestroySVCEncoderFunc(encoder);
//    }
//
//    free(yuvBuffer);
//}
//
//static int frame = 0;
//
//bool SnpEncoderFFmpeg::openH264EncoderEncode(const uint8_t *framebuffer, uint32_t len) {
//    bool result = true;
//    int res;
//    uint32_t frameSize;
//    SnpPort *outputPort = this->getOutputPort(0);
//    SFrameBSInfo info = {};
//    SSourcePicture pic = {};
//
//    VideoUtil::rgba2Yuv(yuvBuffer, framebuffer, width, height);
//
//    pic.iPicWidth = width;
//    pic.iPicHeight = height;
////  pic.uiTimeStamp
//    pic.iColorFormat = videoFormatI420;
//    pic.iStride[0] = pic.iPicWidth;
//    pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;
//    pic.pData[0] = yuvBuffer;
//    pic.pData[1] = pic.pData[0] + width * height;
//    pic.pData[2] = pic.pData[1] + (width * height >> 2);
//
//    res = encoder->EncodeFrame(&pic, &info);
//
//    if(res == cmResultSuccess && info.eFrameType != videoFrameTypeSkip) {
//        //output bitstream
//        int iLayer;
//        for (iLayer=0; iLayer < info.iLayerNum; iLayer++) {
//            SLayerBSInfo* pLayerBsInfo = &info.sLayerInfo[iLayer];
//
//            int iLayerSize = 0;
//            int iNalIdx = pLayerBsInfo->iNalCount - 1;
//            do {
//                iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
//                --iNalIdx;
//            } while (iNalIdx >= 0);
//
//            bool complete = iLayer == info.iLayerNum-1;
//
//            SnpDataRam ram(pLayerBsInfo->pBsBuf, iLayerSize, complete);
//            outputPort->onData(getPipeId(), &ram);
//        }
//    }
//
//    return result;
//    error:
//    return result;
//}