#ifndef SNPSERVER_VIDEOUTIL_H
#define SNPSERVER_VIDEOUTIL_H

#include <cstdint>

class VideoUtil {
public:
    static void rgba2Yuv(uint8_t *destination, const uint8_t *rgb, int width, int height);
};


#endif //SNPSERVER_VIDEOUTIL_H
