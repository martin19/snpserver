#include "VideoUtil.h"
#include <algorithm>

//void VideoUtil::rgba2Nv12(uint8_t *destination, const uint8_t *rgba, int width, int height, int strideY, int strideUV) {
//    int image_size = strideY * height;
//    int ypos = 0;
//    int upos = image_size;
//    int vpos = image_size + 1;
//    int i = 0;
//    int line;
//    int x;
//    for( line = 0; line < height; ++line ) {
//        if( !(line % 2) ) {
//            //even lines Y,U,V
//            for( x = 0; x < width; x += 2 ) {
//                uint8_t r = rgba[4 * i];
//                uint8_t g = rgba[4 * i + 1];
//                uint8_t b = rgba[4 * i + 2];
//                destination[ypos++] = ((66*r + 129*g + 25*b) >> 8) + 16;
//                destination[upos] = ((-38*r + -74*g + 112*b) >> 8) + 128;
//                destination[vpos] = ((112*r + -94*g + -18*b) >> 8) + 128;
//                i++;
//                upos += 2;
//                vpos += 2;
//                r = rgba[4 * i];
//                g = rgba[4 * i + 1];
//                b = rgba[4 * i + 2];
//                destination[ypos++] = ((66*r + 129*g + 25*b) >> 8) + 16;
//                i++;
//            }
//            upos += (strideUV - (width<<1));
//            vpos += (strideUV - (width<<1));
//        } else {
//            //odd lines Y
//            for( x = 0; x < width; x += 1 ) {
//                uint8_t r = rgba[4 * i];
//                uint8_t g = rgba[4 * i + 1];
//                uint8_t b = rgba[4 * i + 2];
//                destination[ypos++] = ((66*r + 129*g + 25*b) >> 8) + 16;
//                i++;
//            }
//        }
//        ypos += (strideY - width);
//    }
//}

void VideoUtil::rgba2Nv12(uint8_t *dst, const uint8_t *rgba, int width, int height, int strideY, int strideC) {
    uint8_t* dstUV = dst + height*strideY;
    for(int y = 0; y < height; y++) {
        auto lineRgba = (uint8_t*)(rgba + y * (width<<2));
        auto lineY = (uint8_t*)(dst + y * strideY);
        auto lineU = (uint8_t*)dstUV + (y>>1) * strideC;
        auto lineV = lineU+1;
        if(y%2 == 0) {
            for(int x = 0; x < width; x+=2) {
                uint8_t r = *lineRgba++;
                uint8_t g = *lineRgba++;
                uint8_t b = *lineRgba++;
                lineRgba++;
                *lineY++ = ((66*r + 129*g + 25*b) >> 8) + 16;
                *lineU = ((-38*r + -74*g + 112*b) >> 8) + 128;
                *lineV = ((112*r + -94*g + -18*b) >> 8) + 128;
                lineU += 2;
                lineV += 2;

                r = *lineRgba++;
                g = *lineRgba++;
                b = *lineRgba++;
                lineRgba++;
                *lineY++ = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        } else {
            for(int x = 0; x < width; x++) {
                uint8_t r = *lineRgba++;
                uint8_t g = *lineRgba++;
                uint8_t b = *lineRgba++;
                lineRgba++;
                *lineY++ = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
    }
}

//matrix values source: https://learn.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
void VideoUtil::rgba2Yuv(uint8_t *destination, const uint8_t *rgb, int width, int height)
{
    int image_size = width * height;
    int upos = image_size;
    int vpos = upos + upos / 4;
    int i = 0;
    int y;
    int x;

    for(y = 0; y < height; ++y ) {
        if( !(y % 2) ) {
            for( x = 0; x < width; x += 2 ) {
                uint8_t r = rgb[4 * i];
                uint8_t g = rgb[4 * i + 1];
                uint8_t b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
                destination[upos++] = ((-38*r + -74*g + 112*b) >> 8) + 128;
                destination[vpos++] = ((112*r + -94*g + -18*b) >> 8) + 128;

                r = rgb[4 * i];
                g = rgb[4 * i + 1];
                b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        } else {
            for( x = 0; x < width; x += 1 ) {
                uint8_t r = rgb[4 * i];
                uint8_t g = rgb[4 * i + 1];
                uint8_t b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
    }
}

void VideoUtil::yuv420ToRgba(uint8_t *rgb, unsigned char **yuv, int width, int height, int strideY, int strideC) {
    int ypos = 0;
    int cofs = 0;

    int i = 0;
    int y;
    int x;

    for(y = 0; y < height; y++ ) {
        for (x = 0; x < width; x++) {

            ypos = (y * strideY + x);
            cofs = ((y >> 1) * strideC + (x >> 1));

            int C = yuv[0][ypos] - 16;
            int D = yuv[1][cofs] - 128;
            int E = yuv[2][cofs] - 128;

            rgb[i++] = std::clamp(( 298 * C           + 409 * E + 128) >> 8, 0, 255);
            rgb[i++] = std::clamp(( 298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255);
            rgb[i++] = std::clamp(( 298 * C + 516 * D           + 128) >> 8, 0, 255);
            rgb[i++] = 255;
        }
    }
}

void VideoUtil::nv12ToRgba(uint8_t *rgba, uint8_t *planeY, uint8_t *planeUV, int width, int height, int strideY, int strideUV) {
    for(uint32_t y = 0; y < height; y++) {
        uint8_t *lineY = planeY + y * strideY;
        uint8_t *lineUV = planeUV + (y >> 1) * strideUV;
        uint8_t *lineRGBA = rgba + y * width * 4;
        for(uint32_t x = 0; x < width; x++) {
            int C = lineY[x] - 16;
            int D = lineUV[((x>>1)<<1)    ] - 128;
            int E = lineUV[((x>>1)<<1) + 1] - 128;

            *lineRGBA++ = std::clamp(( 298 * C           + 409 * E + 128) >> 8, 0, 255);
            *lineRGBA++ = std::clamp(( 298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255);
            *lineRGBA++ = std::clamp(( 298 * C + 516 * D           + 128) >> 8, 0, 255);
            *lineRGBA++ = 255;
        }
    }
}

//void VideoUtil::rgba2Nv12_2(uint8_t *dst, const uint8_t *rgba, int width, int height, int strideY, int strideUV) {
//    uint8_t *dstUV = dst + strideY * height;
//    for(uint32_t y = 0; y < height; y++) {
//        uint8_t *lineRgba = (uint8_t*)rgba + y * (width << 2);
//        uint8_t *lineY = dst + y * strideY;
//        if(y % 2 == 0) {
//            for(uint32_t x = 0; x < width; x++) {
//                uint8_t r = *lineRgba++;
//                uint8_t g = *lineRgba++;
//                uint8_t b = *lineRgba++;
//                lineRgba++;
//                *lineY++ = ((66*r + 129*g + 25*b) >> 8) + 16;
//                if(x % 2 == 0) {
//
//                } else {
//
//                }
//            }
//        } else {
//            for(uint32_t x = 0; x < width; x++) {
//                uint8_t r = *lineRgba++;
//                uint8_t g = *lineRgba++;
//                uint8_t b = *lineRgba++;
//                lineRgba++;
//                *lineY++ = ((66*r + 129*g + 25*b) >> 8) + 16;
//            }
//        }
//
//    }
//}
