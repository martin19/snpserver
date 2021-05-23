#include "common.h"
#include <rfb/rfb.h>
#include "x264.h"

static x264_param_t param;
static x264_t *x264;
static u_char *yuv_buffer = NULL;

static rfbBool initX264(rfbClientPtr cl) {
    rfbBool result = TRUE;
    if(cl->screen->depth != 32) {
        rfbErr("screen depth of 32 bits required for x264 encoding.\n");
        result = FALSE;
        goto error;
    }

    x264_param_default_preset(&param, "ultrafast", "zerolatency");

    param.i_csp = X264_CSP_I420;
    //param.i_csp = X264_CSP_BGR;
    param.i_threads = 1;
    param.i_width = cl->screen->width;
    param.i_height = cl->screen->height;
    param.i_fps_num = 60;
    param.i_fps_den = 1;
    //param.analyse.i_me_range = 8;
    //param.rc.i_bitrate = 8000;

    // Intra refres:
//    param.i_keyint_max = 300;
    param.b_intra_refresh = 1;

    //rate control
//    param.rc.i_qp_constant = 18*3;
//    param.rc.i_qp_min = 18*3;
//    param.rc.i_qp_max = 18*3;

    //x264_param_apply_profile(&param, "baseline");


    x264 = x264_encoder_open(&param);
error:
    return result;
}

rfbBool rfbSendFrameEncodingX264(rfbClientPtr cl) {
    rfbBool result = TRUE;
    x264_nal_t *header_nals;
    int header_nal_count = 0;
    x264_nal_t *image_nals;
    int image_nal_count = 0;
    int w = cl->screen->width;
    int h = cl->screen->height;

    if(x264 == NULL) {
        if(!initX264(cl)) {
            result = FALSE;
            goto error;
        }
        int size = x264_encoder_headers(x264, &header_nals, &header_nal_count);
    } else {
        header_nal_count = 0;
    }

    if(yuv_buffer == NULL) {
        yuv_buffer = calloc(1, w*h + ((w*h)/2));
    }

    if(cl->screen->width != param.i_width || cl->screen->height != param.i_height) {
        //resize input buffer for X264 instance.
    }

    x264_picture_t pic_in = {0};
    x264_picture_init(&pic_in);
    x264_picture_t pic_out = {0};
    x264_picture_init(&pic_out);

    //picIn.i_pts = //set presentation timestamp
    //picIn.prop = //set additional known properties for the image to optimize encoding
    //TODO: seems there is no colorspace conversion in x264 - we need to do it
    //  ourself or send I444 which is supported by HIGH profile - however we don't know
    //  if the android decoder does support it. probably go for the conversion here.

    //framebufferToYUV(cl, yuv_buffer);
    rgba2Yuv(yuv_buffer, (u_char*)cl->screen->frameBuffer, w, h);

    pic_in.img.i_csp = X264_CSP_I420;
    pic_in.img.i_plane = 1;
    pic_in.img.i_stride[0] = w;
    pic_in.img.i_stride[1] = w/2;
    pic_in.img.i_stride[2] = w/2;
    pic_in.img.plane[0] = yuv_buffer;
    pic_in.img.plane[1] = yuv_buffer + (w*h);
    pic_in.img.plane[2] = yuv_buffer + (w*h) + (w*(h))/4;

    int size = x264_encoder_encode(x264, &image_nals, &image_nal_count, &pic_in, &pic_out);
    //rfbLog("x264: encoding image (%d bytes), num_nals=%d\n", size, image_nal_count);
    int i;
    for(i = 0; i < image_nal_count; i++) {
        //rfbLog("x264: size nal[%d] = %d\n",i, image_nals[i].i_payload);
    }

    if(!sendFramebufferUpdateMsg(cl, 0, 0, w, h, header_nals, header_nal_count, image_nals, image_nal_count)) {
        rfbErr("x264: sendUpdateRect failed.");
    }
error:
    return result;
}

void rfbH264Cleanup(rfbScreenInfoPtr screen) {
    x264_encoder_close(x264);
    free(yuv_buffer);
}