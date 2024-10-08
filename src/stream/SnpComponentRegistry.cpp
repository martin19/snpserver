#include "SnpComponentRegistry.h"
#include "util/loguru.h"
#include "network/snappyv1.pb.h"

SnpComponentRegistry::SnpComponentRegistry() {
    registerLocalComponents();
}

void SnpComponentRegistry::registerLocalComponents() {
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_CAPTURE_VIDEO_DUMMY);
        registerLocalComponent(c);
    }

    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_OUTPUT_VIDEO_DISPLAY);
        registerLocalComponent(c);
    }
#ifdef HAVE_LIBDRM
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_CAPTURE_VIDEO_DRM);
        registerComponent(c);
    }
#endif
#ifdef HAVE_LIBX11
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_CAPTURE_VIDEO_X11);
        registerComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_INPUT_KEYBOARD_X11);
        registerComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_INPUT_POINTER_X11);
        registerComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_INPUT_CURSOR_X11);
        registerComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_OUTPUT_KEYBOARD_X11);
        registerComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_OUTPUT_POINTER_X11);
        registerComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_OUTPUT_CURSOR_X11);
        registerComponent(c);
    }
#endif
//    registerComponent(snappyv1::COMPONENT_CAPTURE_VIDEO_WAYLAND);
//    registerComponent(snappyv1::COMPONENT_CAPTURE_VIDEO_V4L);
#ifdef HAVE_LIBOPENH264
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_ENCODER_OPENH264);
        registerLocalComponent(c);
    }
    {
        auto c = new snappyv1::Component();
        c->set_componenttype(snappyv1::COMPONENT_DECODER_OPENH264);
        registerLocalComponent(c);
    }
#endif
#ifdef HAVE_LIBVA
    registerComponent(snappyv1::COMPONENT_ENCODER_INTEL);
    registerComponent(snappyv1::COMPONENT_DECODER_INTEL);
#endif
//    registerComponent(snappyv1::COMPONENT_ENCODER_AMD );
//    registerComponent(snappyv1::COMPONENT_DECODER_AMD );
}

void SnpComponentRegistry::registerLocalComponent(snappyv1::Component* component) {
    localComponents.insert(component);
}

bool SnpComponentRegistry::hasLocalComponent(snappyv1::Component* component) {
    return localComponents.contains(component);
}

std::set<snappyv1::Component*> SnpComponentRegistry::getLocalComponents() {
    return localComponents;
}

std::set<snappyv1::Component *> SnpComponentRegistry::getRemoteComponents() {
    return remoteComponents;
}

void SnpComponentRegistry::registerRemoteComponent(snappyv1::Component *component) {
    remoteComponents.insert(component);
}

bool SnpComponentRegistry::hasRemoteComponent(snappyv1::Component *component) {
    return remoteComponents.contains(component);
}

void SnpComponentRegistry::registerRemoteComponents(snappyv1::Message *message) {
    auto capabilities = message->capabilities();
    for(int i = 0; i < capabilities.component_size(); i++) {
        auto component = new snappyv1::Component(capabilities.component(i));
        registerRemoteComponent(component);
    }
}


