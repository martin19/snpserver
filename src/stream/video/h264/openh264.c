#include "codec_api.h"
#include "common.h"
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
//#include "linux/dma-buf.h"

static ISVCEncoder *encoder = NULL;
static u_char *yuv_buffer = NULL;
static u_int64_t t0;

static uint8_t *initDmaBuf() {
    int dma_buf_fd = -1;
//    uint32_t fb_id = 0x53;
//    uint32_t fb_id = 0x38;
//    uint32_t fb_id = 0x44; //devbox
//    uint32_t fb_id = 0xce; //raspberry
    uint32_t fb_id = 0x63; //sat
//    uint32_t fb_id = 0x65; //sat

    char *card = "/dev/dri/card0";
    const int drmfd = open(card, O_RDWR);
    if (drmfd < 0) {
        fprintf(stderr, "Cannot open card\n");
        goto error;
    }
    drmModeFBPtr fb = drmModeGetFB(drmfd, fb_id);
    if (!fb) {
        fprintf(stderr, "Cannot open fb %#x\n", fb_id);
        goto error;
    }

    printf("fb_id=%#x width=%u height=%u pitch=%u bpp=%u depth=%u handle=%#x\n",
           fb_id, fb->width, fb->height, fb->pitch, fb->bpp, fb->depth, fb->handle);

    const int ret = drmPrimeHandleToFD(drmfd, fb->handle, DRM_RDWR, &dma_buf_fd);
    printf("drmPrimeHandleToFD = %d, fd = %d\n", ret, dma_buf_fd);

//    /* open mapped file */
//    fid = open( "/var/tmp/Xvfb_screen0", O_RDWR, S_IRUSR | S_IWUSR);
//    if (fid < 0) {
//        fprintf(stderr, "Error: cannot open /var/tmp/Xvfb_screen0");
//        exit(1);
//    }

    uint8_t *map = NULL;
    map = (uint8_t*)mmap(NULL, fb->width*fb->height*4, PROT_READ, MAP_SHARED, dma_buf_fd, 0);
    if (map == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        goto error;
    }

    return map;

error:
    if (dma_buf_fd >= 0)
        close(dma_buf_fd);
    if (fb)
        drmModeFreeFB(fb);
    close(drmfd);
    return 0;
}

static rfbBool initOpenH264(rfbClientPtr cl) {
    rfbBool result = TRUE;
    if(cl->screen->depth != 32) {
        rfbErr("screen depth of 32 bits required for x264 encoding.\n");
        result = FALSE;
        goto error;
    }

    cl->screen->frameBuffer = (char *)initDmaBuf();

    WelsCreateSVCEncoder(&encoder);
//    SEncParamBase param;
//    memset (&param, 0, sizeof (SEncParamBase));
////    param.iUsageType = SCREEN_CONTENT_REAL_TIME; //from EUsageType enum
//    param.iUsageType = CAMERA_VIDEO_REAL_TIME; //from EUsageType enum
//    param.fMaxFrameRate = 60;
//    param.iPicWidth = cl->screen->width;
//    param.iPicHeight = cl->screen->height;
//    param.iTargetBitrate = 20000000;
//    (*encoder)->Initialize(encoder, &param);
//    printf("after initEncoder\n");

    SEncParamExt paramExt;
    memset (&paramExt, 0, sizeof (SEncParamExt));
//    paramExt.iUsageType = CAMERA_VIDEO_REAL_TIME;
    paramExt.bEnableFrameSkip = 1;
    paramExt.iNumRefFrame = 1;
    paramExt.fMaxFrameRate = 60;
    paramExt.iPicWidth = cl->screen->width;
    paramExt.iPicHeight = cl->screen->height;
    paramExt.iTargetBitrate = 20000000;
    paramExt.bEnableFrameSkip = 1;
    paramExt.iMinQp = 15;
    paramExt.iMaxQp = 15;
    (*encoder)->InitializeExt(encoder, &paramExt);
//    paramExt.iSpatialLayerNum = 2;
//    paramExt.iNumRefFrame = 1;
//
//    paramExt.sSpatialLayers[0].iVideoWidth = cl->screen->width;
//    paramExt.sSpatialLayers[0].iVideoHeight = cl->screen->height/2;
//    paramExt.sSpatialLayers[0].fFrameRate = 30;
//    paramExt.sSpatialLayers[0].iSpatialBitrate = 15000000/2;
//    paramExt.sSpatialLayers[1].iVideoWidth = cl->screen->width;
//    paramExt.sSpatialLayers[1].iVideoHeight = cl->screen->height/2;
//    paramExt.sSpatialLayers[1].fFrameRate = 30;
//    paramExt.sSpatialLayers[1].iSpatialBitrate = 15000000/2;
//    encoder->InitializeExt(&paramExt);
error:
    return result;
}

extern rfbBool rfbSendFrameEncodingOpenH264(rfbClientPtr cl) {
    rfbBool result = TRUE;
    int w = cl->screen->width;
    int h = cl->screen->height;
    int rv;
    int frameSize;

    if(encoder == NULL) {
        if(!initOpenH264(cl)) {
            cl->rfbStatistics.last_frame = 0;
            result = FALSE;
            goto error;
        }
        t0 = getTimeNowMs();
    } else {
        cl->rfbStatistics.last_frame++;
    }

    usleep(16666/2);
    if(yuv_buffer == NULL) {
        yuv_buffer = (u_char*)malloc(w*h + ((w*h)/2));
    }

//    if(cl->screen->width != w || cl->screen->height != param.i_height) {
//        //resize input buffer for X264 instance.
//    }

    cl->rfbStatistics.encode_ts_start_ms = (uint32_t)(getTimeNowMs() - t0);

    rgba2Yuv(yuv_buffer, (u_char*)cl->screen->frameBuffer, w, h);

    frameSize = w * h * 3 / 2;
    SFrameBSInfo info;
    memset (&info, 0, sizeof (SFrameBSInfo));
    SSourcePicture pic;
    memset (&pic, 0, sizeof (SSourcePicture));
    pic.iPicWidth = w;
    pic.iPicHeight = h;
//    pic.uiTimeStamp
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = pic.iPicWidth;
    pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;
    pic.pData[0] = yuv_buffer;
    pic.pData[1] = pic.pData[0] + w * h;
    pic.pData[2] = pic.pData[1] + (w * h >> 2);
    printf("encodeFrame %d\n",cl->rfbStatistics.last_frame);
    rv = (*encoder)->EncodeFrame(encoder, &pic, &info);

    cl->rfbStatistics.encode_ts_end_ms = (uint32_t)(getTimeNowMs() - t0);

    cl->rfbStatistics.tx_ts_start_ms = (uint32_t)(getTimeNowMs() - t0);

    if(rv == cmResultSuccess && info.eFrameType != videoFrameTypeSkip) {
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

            unsigned char *outBuf = pLayerBsInfo->pBsBuf;
            sendFramebufferUpdateMsg(cl, 0, 0, w, h, outBuf, iLayerSize, 1);
        }
    }

    cl->rfbStatistics.tx_ts_end_ms = (uint32_t)(getTimeNowMs() - t0);
    rfbSendStatistics(cl);

error:
    return result;
}

void rfbH264Cleanup(rfbScreenInfoPtr screen) {
    if (encoder) {
        (*encoder)->Uninitialize(encoder);
        WelsDestroySVCEncoder(encoder);
    }
    free(yuv_buffer);
}