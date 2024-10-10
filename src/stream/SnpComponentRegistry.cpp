#include "SnpComponentRegistry.h"
#include "util/loguru.h"
#include "network/snp.pb.h"

SnpComponentRegistry::SnpComponentRegistry() {
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


