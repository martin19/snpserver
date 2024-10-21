#ifndef SNPSERVER_VIDEOUTIL_H
#define SNPSERVER_VIDEOUTIL_H

#include <cstdint>

class VideoUtil {
public:
    static void rgba2Nv12(uint8_t *destination, const uint8_t *rgb, int width, int height, int strideY, int strideUV);
    static void rgba2Yuv(uint8_t *destination, const uint8_t *rgb, int width, int height);
    static void yuv420ToRgba(uint8_t *rgb, unsigned char **yuv, int width, int height, int strideY, int strideC);
    static void nv12ToRgba(uint8_t *rgba, uint8_t *y, uint8_t *uv, int width, int height, int strideY, int strideUV);
};


#endif //SNPSERVER_VIDEOUTIL_H
