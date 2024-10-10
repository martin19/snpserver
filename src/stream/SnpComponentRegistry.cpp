#include "SnpComponentRegistry.h"
#include "util/loguru.h"
#include "network/snp.pb.h"

SnpComponentRegistry::SnpComponentRegistry() {
    componentMap = {
            {"COMPONENT_CAPTURE_VIDEO_DRM", 0},
            {"COMPONENT_CAPTURE_VIDEO_X11", 1},
            {"COMPONENT_CAPTURE_VIDEO_WAYLAND", 2},
            {"COMPONENT_CAPTURE_VIDEO_V4L", 3},
            {"COMPONENT_CAPTURE_VIDEO_DUMMY", 4},
            {"COMPONENT_INPUT_KEYBOARD_X11", 5},
            {"COMPONENT_INPUT_POINTER_X11", 6},
            {"COMPONENT_INPUT_CURSOR_X11", 7},
            {"COMPONENT_OUTPUT_VIDEO_DISPLAY", 8},
            {"COMPONENT_OUTPUT_KEYBOARD_X11", 9},
            {"COMPONENT_OUTPUT_POINTER_X11", 10},
            {"COMPONENT_OUTPUT_CURSOR_X11", 11},
            {"COMPONENT_ENCODER_OPENH264", 12},
            {"COMPONENT_DECODER_OPENH264", 13},
            {"COMPONENT_ENCODER_INTEL", 14},
            {"COMPONENT_DECODER_INTEL", 15},
            {"COMPONENT_ENCODER_AMD", 16},
            {"COMPONENT_DECODER_AMD", 17},
            {"COMPONENT_OUTPUT_FILE", 18},
            {"COMPONENT_OUTPUT_TCP", 19},
            {"COMPONENT_INPUT_TCP", 20},
            {"COMPONENT_OUTPUT_WEBSOCKET", 21},
            {"COMPONENT_INPUT_WEBSOCKET", 22},
            {"COMPONENT_ENCODER_MMAL", 23},
            {"COMPONENT_DECODER_MMAL", 24},
            {"COMPONENT_CAPTURE_GL", 25}
    };

    registerLocalComponents();
}

void SnpComponentRegistry::registerLocalComponents() {
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_CAPTURE_VIDEO_DUMMY);
        registerLocalComponent(c);
    }

    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_OUTPUT_VIDEO_DISPLAY);
        registerLocalComponent(c);
    }
#ifdef HAVE_LIBDRM
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_CAPTURE_VIDEO_DRM);
        registerComponent(c);
    }
#endif
#ifdef HAVE_LIBX11
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_CAPTURE_VIDEO_X11);
        registerComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_INPUT_KEYBOARD_X11);
        registerComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_INPUT_POINTER_X11);
        registerComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_INPUT_CURSOR_X11);
        registerComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_OUTPUT_KEYBOARD_X11);
        registerComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_OUTPUT_POINTER_X11);
        registerComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_OUTPUT_CURSOR_X11);
        registerComponent(c);
    }
#endif
//    registerComponent(snp::COMPONENT_CAPTURE_VIDEO_WAYLAND);
//    registerComponent(snp::COMPONENT_CAPTURE_VIDEO_V4L);
#ifdef HAVE_LIBOPENH264
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_ENCODER_OPENH264);
        registerLocalComponent(c);
    }
    {
        auto c = new snp::Component();
        c->set_componenttype(snp::COMPONENT_DECODER_OPENH264);
        registerLocalComponent(c);
    }
#endif
#ifdef HAVE_LIBVA
    registerComponent(snp::COMPONENT_ENCODER_INTEL);
    registerComponent(snp::COMPONENT_DECODER_INTEL);
#endif
//    registerComponent(snp::COMPONENT_ENCODER_AMD );
//    registerComponent(snp::COMPONENT_DECODER_AMD );
}

void SnpComponentRegistry::registerLocalComponent(snp::Component* component) {
    localComponents.insert(component);
}

bool SnpComponentRegistry::hasLocalComponent(snp::Component* component) {
    return localComponents.contains(component);
}

std::set<snp::Component*> SnpComponentRegistry::getLocalComponents() {
    return localComponents;
}

std::set<snp::Component *> SnpComponentRegistry::getRemoteComponents() {
    return remoteComponents;
}

void SnpComponentRegistry::registerRemoteComponent(snp::Component *component) {
    remoteComponents.insert(component);
}

bool SnpComponentRegistry::hasRemoteComponent(snp::Component *component) {
    return remoteComponents.contains(component);
}

void SnpComponentRegistry::registerRemoteComponents(snp::Message *message) {
    auto capabilities = message->capabilities();
    for(int i = 0; i < capabilities.component_size(); i++) {
        auto component = new snp::Component(capabilities.component(i));
        registerRemoteComponent(component);
    }
}

int SnpComponentRegistry::getComponentId(const std::string& componentName) const {
    auto it = componentMap.find(componentName);
    if (it != componentMap.end()) {
        return it->second;
    } else {
        return -1; // Return -1 if component name not found
    }
}


