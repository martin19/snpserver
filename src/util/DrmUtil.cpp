#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "DrmUtil.h"

DrmUtil::DrmUtil(int deviceFd) {
    this->deviceFd = deviceFd;
    getResources();
}

DrmUtil::~DrmUtil() {
    freeResources();
}

//bool DrmUtil::openDevice(const std::string &device) {
//    bool result = true;
//
//    deviceFd = open(device.c_str(), O_RDWR);
//    if (deviceFd < 0) {
//        result = false;
//        fprintf(stderr, "Cannot open modesetting device %s\n", device.c_str());
//        goto error;
//    }
//
//    return result;
//error:
//    return result;
//}

void DrmUtil::freeResources() {
    for(auto & connector : connectors) {
        for(auto & property : connector.second->props) {
            drmModeFreeProperty(property.second.spec);
        }
        drmModeFreeConnector(connector.second->ptr);
        delete connector.second;
    }
    for(auto & encoder : encoders) {
        for(auto & property : encoder.second->props) {
            drmModeFreeProperty(property.second.spec);
        }
        drmModeFreeEncoder(encoder.second->ptr);
        delete encoder.second;
    }
    for(auto & crtc : crtcs) {
        for(auto & property : crtc.second->props) {
            drmModeFreeProperty(property.second.spec);
        }
        drmModeFreeCrtc(crtc.second->ptr);
        delete crtc.second;
    }
    for(auto & plane : planes) {
        for(auto & property : plane.second->props) {
            drmModeFreeProperty(property.second.spec);
        }
        drmModeFreePlane(plane.second->ptr);
        delete plane.second;
    }
    for(auto & framebuffer : framebuffers) {
        for(auto & property : framebuffer.second->props) {
            drmModeFreeProperty(property.second.spec);
        }
        drmModeFreeFB(framebuffer.second->ptr);
        delete framebuffer.second;
    }
}

bool DrmUtil::getResources() {
    bool result = true;

    drmSetClientCap(deviceFd, DRM_CLIENT_CAP_ATOMIC, 1);
    drmSetClientCap(deviceFd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
//    drmSetClientCap(deviceFd, DRM_CLIENT_CAP_WRITEBACK_CONNECTORS, 1);

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
        auto properties = drmModeObjectGetProperties(deviceFd, connector.first, DRM_MODE_OBJECT_CONNECTOR);
        if(!properties) continue;
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            connector.second->props.insert(std::pair<std::string, DrmProperty>(std::string(prop->name),
                { .spec=prop, .value=properties->prop_values[i] }));
        }
    }
    for(auto & encoder : encoders) {
        auto properties = drmModeObjectGetProperties(deviceFd, encoder.first, DRM_MODE_OBJECT_ENCODER);
        if(!properties) continue;
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            encoder.second->props.insert(std::pair<std::string, DrmProperty>(std::string(prop->name),
                                                                               { .spec=prop, .value=properties->prop_values[i] }));
        }
    }
    for(auto & crtc : crtcs) {
        auto properties = drmModeObjectGetProperties(deviceFd, crtc.first, DRM_MODE_OBJECT_CRTC);
        if(!properties) continue;
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            crtc.second->props.insert(std::pair<std::string, DrmProperty>(std::string(prop->name),
                                                                             { .spec=prop, .value=properties->prop_values[i] }));
        }
    }
    for(auto & framebuffer : framebuffers) {
        auto properties = drmModeObjectGetProperties(deviceFd, framebuffer.first, DRM_MODE_OBJECT_FB);
        if(!properties) continue;
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            framebuffer.second->props.insert(std::pair<std::string, DrmProperty>(std::string(prop->name),
                                                                             { .spec=prop, .value=properties->prop_values[i] }));
        }
    }
    for(auto & plane : planes) {
        auto properties = drmModeObjectGetProperties(deviceFd, plane.first, DRM_MODE_OBJECT_PLANE);
        if(!properties) continue;
        for(int i = 0; i < properties->count_props; i++) {
            auto prop = drmModeGetProperty(deviceFd, properties->props[i]);
            plane.second->props.insert(std::pair<std::string, DrmProperty>(std::string(prop->name),
                                                                             { .spec=prop, .value=properties->prop_values[i] }));
        }
    }

    return result;
error:
    return result;
}

bool DrmUtil::getPrimaryFb(uint32_t *fbId) {
    bool result = true;
    uint32_t connectorId = -1;
    uint64_t crtcId = -1;
    uint64_t planeId = -1;

    if(!deviceFd) {
        result = false;
        fprintf(stderr, "deviceFd is not open. Cannot determine primary framebuffer.");
        goto error;
    }

    //determine crtc of first active connector
    for(auto & connector : connectors) {
        if(connector.second->ptr->connection == DRM_MODE_CONNECTED) {
            auto properties = connector.second->props;
            auto property = properties.find("CRTC_ID");
            crtcId = property->second.value;
            break;
        }
    }

    if(crtcId == -1) {
        result = false;
        fprintf(stderr, "cannot find connected crtc.");
        goto error;
    }

    //find primary plane of crtc
    for(auto & plane : planes) {
        if(plane.second->ptr->crtc_id) {
            auto properties = plane.second->props;
            auto property = properties.find("type");
            if(property->second.value == 1) {
                planeId = plane.first;
            }
//            if(property->enumValue().equals("primary")) {
//                planeId = plane.first;
//            }
        }
    }

    if(crtcId == -1) {
        result = false;
        fprintf(stderr, "cannot find primary plane.");
        goto error;
    }

//TODO:
//    std::iterator<std::pair<uint32_t, Plane*>> it = planes.find(planeId);
//    if(it != planes.end()) {
//        result = false;
//        fprintf(stderr, "cannot find primary framebuffer.");
//        goto error;
//    }

    *fbId = planes.find(planeId)->second->ptr->fb_id;

    std::cout << "Found primary framebuffer " << *fbId << std::endl;

    return result;
error:
    return result;
}
