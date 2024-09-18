#ifndef SNPSERVER_VIDEOUTIL_H
#define SNPSERVER_VIDEOUTIL_H

#include <cstdint>

class VideoUtil {
public:
    static void rgba2NV1(uint8_t *destination, const uint8_t *rgb, uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight);
    static void rgba2Yuv(uint8_t *destination, const uint8_t *rgb, int width, int height);
    static void yuv420ToRgb(uint8_t *rgb, const uint8_t *yuv, int width, int height);
};


#endif //SNPSERVER_VIDEOUTIL_H
