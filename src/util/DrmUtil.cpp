#include <fcntl.h>
#include <unistd.h>
#include "DrmUtil.h"

DrmUtil::DrmUtil(int deviceFd) {
    this->deviceFd = deviceFd;
    getResources();
}

DrmUtil::DrmUtil(std::string device) {
    this->deviceFd = -1;
    if(openDevice(device)) {
        getResources();
    }
}

DrmUtil::~DrmUtil() {
    freeResources();
    if(deviceFd) {
        close(deviceFd);
    }
    deviceFd = -1;
}

bool DrmUtil::openDevice(const std::string &device) {
    bool result = true;

    deviceFd = open(device.c_str(), O_RDWR);
    if (deviceFd < 0) {
        result = false;
        fprintf(stderr, "Cannot open modesetting device %s\n", device.c_str());
        goto error;
    }

    return result;
error:
    return result;
}

void DrmUtil::closeDevice() {
    close(deviceFd);
}

void DrmUtil::freeResources() {
    for(auto & connector : connectors) {
        for(auto & property : connector.second->properties) {
            drmModeFreeProperty(property.second);
        }
        drmModeFreeConnector(connector.second->ptr);
        delete connector.second;
    }
    for(auto & encoder : encoders) {
        for(auto & property : encoder.second->properties) {
            drmModeFreeProperty(property.second);
        }
        drmModeFreeEncoder(encoder.second->ptr);
        delete encoder.second;
    }
    for(auto & crtc : crtcs) {
        for(auto & property : crtc.second->properties) {
            drmModeFreeProperty(property.second);
        }
        drmModeFreeCrtc(crtc.second->ptr);
        delete crtc.second;
    }
    for(auto & plane : planes) {
        for(auto & property : plane.second->properties) {
            drmModeFreeProperty(property.second);
        }
        drmModeFreePlane(plane.second->ptr);
        delete plane.second;
    }
    for(auto & framebuffer : framebuffers) {
        for(auto & property : framebuffer.second->properties) {
            drmModeFreeProperty(property.second);
        }
        drmModeFreeFB(framebuffer.second->ptr);
        delete framebuffer.second;
    }
}

bool DrmUtil::getResources() {
    bool result = true;

    //enumerate resources
    drmModeResPtr resources = drmModeGetResources(deviceFd);
    for(int i = 0; i < resources->count_connectors; i++) {
        auto connector = new Connector();
        connector->ptr = drmModeGetConnector(deviceFd, resources->connectors[i]);
        connectors.insert(std::pair<uint32_t, Connector*>(resources->connectors[i], connector));
    }
    for(int i = 0; i < resources->count_encoders; i++) {
        auto encoder = new Encoder();
        encoder->ptr = drmModeGetEncoder(deviceFd, resources->encoders[i]);
        encoders.insert(std::pair<uint32_t, Encoder*>(resources->encoders[i], encoder));
    }
    for(int i = 0; i < resources->count_crtcs; i++) {
        auto crtc = new Crtc();
        crtc->ptr = drmModeGetCrtc(deviceFd, resources->crtcs[i]);
        crtcs.insert(std::pair<uint32_t, Crtc*>(resources->crtcs[i], crtc));
    }
    for(int i = 0; i < resources->count_fbs; i++) {
        auto fb = new FB();
        fb->ptr = drmModeGetFB(deviceFd, resources->fbs[i]);
        framebuffers.insert(std::pair<uint32_t, FB*>(resources->fbs[i], fb));
    }
    drmModePlaneResPtr planeResources = drmModeGetPlaneResources(deviceFd);
    for(int i = 0; i < planeResources->count_planes; i++) {
        auto plane = new Plane();
        plane->ptr = drmModeGetPlane(deviceFd, planeResources->planes[i]);
        planes.insert(std::pair<uint32_t, Plane*>(planeResources->planes[i], plane));
    }

    //get all properties
    for(auto & connector : connectors) {
        auto properties = drmModeObjectGetProperties(deviceFd, connector.first, DRM_MODE_OBJECT_CRTC);
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            connector.second->properties.insert(std::pair<std::string, drmModePropertyPtr>(std::string(prop->name), prop));
        }
    }
    for(auto & encoder : encoders) {
        auto properties = drmModeObjectGetProperties(deviceFd, encoder.first, DRM_MODE_OBJECT_ENCODER);
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            encoder.second->properties.insert(std::pair<std::string, drmModePropertyPtr>(std::string(prop->name), prop));
        }
    }
    for(auto & crtc : crtcs) {
        auto properties = drmModeObjectGetProperties(deviceFd, crtc.first, DRM_MODE_OBJECT_CRTC);
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            crtc.second->properties.insert(std::pair<std::string, drmModePropertyPtr>(std::string(prop->name), prop));
        }
    }
    for(auto & framebuffer : framebuffers) {
        auto properties = drmModeObjectGetProperties(deviceFd, framebuffer.first, DRM_MODE_OBJECT_FB);
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            framebuffer.second->properties.insert(std::pair<std::string, drmModePropertyPtr>(std::string(prop->name), prop));
        }
    }
    for(auto & plane : planes) {
        auto properties = drmModeObjectGetProperties(deviceFd, plane.first, DRM_MODE_OBJECT_PLANE);
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            plane.second->properties.insert(std::pair<std::string, drmModePropertyPtr>(std::string(prop->name), prop));
        }
    }

    return result;
error:
    return result;
}

bool DrmUtil::getPrimaryFb(uint32_t *fbId) {
    bool result = true;
    uint32_t crtcId = -1;

    if(!deviceFd) {
        result = false;
        fprintf(stderr, "deviceFd is not open. Cannot determine primary framebuffer.");
        goto error;
    }

    //determine first active crtc
    for(auto & crtc : crtcs) {
        auto properties = crtc.second->properties;
        auto propertyActive = properties.find("ACTIVE");
        //TODO: inspect element
    }

    return result;
error:
    return result;
}
