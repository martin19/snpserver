#ifndef SNPSERVER_VAUTILS_H
#define SNPSERVER_VAUTILS_H


class VaUtils {
public:
    static bool uploadSurfaceYuv(void *vaDisplay, unsigned int surfaceId, int srcFourcc, int srcWidth, int srcHeight,
                          unsigned char *srcY, unsigned char *srcU, unsigned char *srcV);
};


#endif //SNPSERVER_VAUTILS_H
