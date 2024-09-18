#include "VideoUtil.h"
#include <algorithm>

void VideoUtil::rgba2NV1(uint8_t *destination, const uint8_t *rgb, uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight) {
    int image_size = dstWidth * dstHeight;
    int upos = image_size;
    int vpos = image_size + 1;
    int i = 0;
    int line;
    int x;

    for( line = 0; line < srcHeight; ++line )
    {
        if( !(line % 2) )
        {
            for( x = 0; x < srcWidth; x += 2 )
            {
                uint8_t r = rgb[4 * i];
                uint8_t g = rgb[4 * i + 1];
                uint8_t b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;

                destination[upos] = ((-38*r + -74*g + 112*b) >> 8) + 128;
                destination[vpos] = ((112*r + -94*g + -18*b) >> 8) + 128;
                upos += 2;
                vpos += 2;

                r = rgb[4 * i];
                g = rgb[4 * i + 1];
                b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
        else
        {
            for( x = 0; x < srcWidth; x += 1 )
            {
                uint8_t r = rgb[4 * i];
                uint8_t g = rgb[4 * i + 1];
                uint8_t b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
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

void VideoUtil::yuv420ToRgb(uint8_t *rgb, const uint8_t *yuv, int width, int height) {
    int image_size = width * height;
    int ypos = 0;
    int cofs = 0;

    int upos = image_size;
    int vpos = upos + upos / 4;
    int i = 0;
    int y;
    int x;

    for(y = 0; y < height; y++ ) {
        for (x = 0; x < width; x++) {

            ypos = (y * width + x);
            cofs = ((y >> 1) * width + (x >> 1));

            uint8_t C = yuv[ypos] - 16;
            uint8_t D = yuv[upos + cofs] - 128;
            uint8_t E = yuv[vpos + cofs] - 128;

            rgb[i++] = std::clamp(( 298 * C           + 409 * E + 128) >> 8, 0, 255);
            rgb[i++] = std::clamp(( 298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255);
            rgb[i++] = std::clamp(( 298 * C + 516 * D           + 128) >> 8, 0, 255);
        }
    }
}