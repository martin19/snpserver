#ifndef SNPSERVER_DRMUTIL_H
#define SNPSERVER_DRMUTIL_H

#include <xf86drmMode.h>
#include <xf86drm.h>
#include <string>
#include <map>

struct DrmProperty {
    drmModePropertyPtr spec;
    uint64_t value;
};

struct Connector {
    drmModeConnectorPtr ptr;
    std::map<std::string, DrmProperty> props;
};

struct Encoder {
    drmModeEncoderPtr ptr;
    std::map<std::string, DrmProperty> props;
};

struct Crtc {
    drmModeCrtcPtr ptr;
    std::map<std::string, DrmProperty> props;
};

struct Plane {
    drmModePlanePtr ptr;
    std::map<std::string, DrmProperty> props;
};

struct FB {
    drmModeFBPtr ptr;
    std::map<std::string, DrmProperty> props;
};

class DrmUtil {
public:
    explicit DrmUtil(int deviceFd);
    explicit DrmUtil(std::string device);
    virtual ~DrmUtil();
    bool getResources();
    void freeResources();
    bool getPrimaryFb(uint32_t *fbId);
private:
    int deviceFd;
    std::map<uint32_t, Connector*> connectors;
    std::map<uint32_t, Encoder*> encoders;
    std::map<uint32_t, Crtc*> crtcs;
    std::map<uint32_t, Plane*> planes;
    std::map<uint32_t, FB*> framebuffers;

    bool openDevice(const std::string &device);
    void closeDevice();
};


#endif //SNPSERVER_DRMUTIL_H
