#include <cstring>
#include <cassert>
#include "VaUtils.h"
#include "va/va.h"
#include "util/loguru.h"

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        LOG_F(ERROR,"%s failed with status %d,exit\n", __func__, (int)va_status); \
        goto error;                                                        \
    }

/*
 * Upload YUV data from memory into a surface
 * if src_fourcc == NV12, assume the buffer pointed by src_U
 * is UV interleaved (src_V is ignored)
 */
bool VaUtils::uploadSurfaceYuv(VADisplay vaDisplay, VASurfaceID surfaceId,
                            int srcFourcc, int srcWidth, int srcHeight,
                            unsigned char *srcY, unsigned char *srcU, unsigned char *srcV) {
    bool result = true;
    VAStatus status;
    VAImage surface_image;
    unsigned char *surfaceP = nullptr, *Y_start = nullptr, *U_start = nullptr;
    uint32_t Y_pitch = 0, U_pitch = 0, row;

    status = vaDeriveImage(vaDisplay, surfaceId, &surface_image);
    CHECK_VASTATUS(status, "vaDeriveImage");

    vaMapBuffer(vaDisplay, surface_image.buf, (void **)&surfaceP);
    assert(VA_STATUS_SUCCESS == status);

    Y_start = surfaceP;
    Y_pitch = surface_image.pitches[0];
    switch (surface_image.format.fourcc) {
        case VA_FOURCC_NV12:
            U_start = (unsigned char *)surfaceP + surface_image.offsets[1];
            U_pitch = surface_image.pitches[1];
            break;
        case VA_FOURCC_IYUV:
            U_start = (unsigned char *)surfaceP + surface_image.offsets[1];
            U_pitch = surface_image.pitches[1];
            break;
        case VA_FOURCC_YV12:
            U_start = (unsigned char *)surfaceP + surface_image.offsets[2];
            U_pitch = surface_image.pitches[2];
            break;
        case VA_FOURCC_YUY2:
            U_start = surfaceP + 1;
            U_pitch = surface_image.pitches[0];
            break;
        default:
            assert(0);
    }

    /* copy Y plane */
    for (row = 0; row < srcHeight; row++) {
        unsigned char *Y_row = Y_start + row * Y_pitch;
        memcpy(Y_row, srcY + row * srcWidth, srcWidth);
    }

    for (row = 0; row < srcHeight / 2; row++) {
        unsigned char *U_row = U_start + row * U_pitch;
        unsigned char *u_ptr = NULL, *v_ptr = NULL;
        int j;
        switch (surface_image.format.fourcc) {
            case VA_FOURCC_NV12:
                if (srcFourcc == VA_FOURCC_NV12) {
                    memcpy(U_row, srcU + row * srcWidth, srcWidth);
                    break;
                } else if (srcFourcc == VA_FOURCC_IYUV) {
                    u_ptr = srcU + row * (srcWidth / 2);
                    v_ptr = srcV + row * (srcWidth / 2);
                } else if (srcFourcc == VA_FOURCC_YV12) {
                    v_ptr = srcU + row * (srcWidth / 2);
                    u_ptr = srcV + row * (srcWidth / 2);
                }
                if ((srcFourcc == VA_FOURCC_IYUV) ||
                    (srcFourcc == VA_FOURCC_YV12)) {
                    for (j = 0; j < srcWidth / 2; j++) {
                        U_row[2 * j] = u_ptr[j];
                        U_row[2 * j + 1] = v_ptr[j];
                    }
                }
                break;
            case VA_FOURCC_IYUV:
            case VA_FOURCC_YV12:
            case VA_FOURCC_YUY2:
            default: {
                LOG_F(ERROR, "unsupported fourcc in load_surface_yuv\n");
                return false;
            }
        }
    }

    vaUnmapBuffer(vaDisplay, surface_image.buf);

    vaDestroyImage(vaDisplay, surface_image.image_id);

    return result;
error:
    return result;
}