#include "VideoUtil.h"


void VideoUtil::rgba2Yuv(uint8_t *destination, const uint8_t *rgb, int width, int height)
{
    int image_size = width * height;
    int upos = image_size;
    int vpos = upos + upos / 4;
    int i = 0;
    int line;
    int x;

    for( line = 0; line < height; ++line )
    {
        if( !(line % 2) )
        {
            for( x = 0; x < width; x += 2 )
            {
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
        }
        else
        {
            for( x = 0; x < width; x += 1 )
            {
                uint8_t r = rgb[4 * i];
                uint8_t g = rgb[4 * i + 1];
                uint8_t b = rgb[4 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
    }
}