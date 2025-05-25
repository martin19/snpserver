#include <vector>
#include "util/loguru.h"
#include "SnpPipeFactory.h"
#include "SnpPipe.h"
#include "stream/video/SnpSourceDummy.h"
#include "stream/output/SnpSinkDisplay.h"
#ifdef HAVE_LIBGL
    #include <stream/video/SnpSourceGL.h>
#endif //HAVE_LIBGL
#ifdef HAVE_LIBVA
    #include <stream/video/SnpEncoderVaH264.h>
#endif //HAVE_LIBVA
#ifdef HAVE_LIBX11
    #include <stream/video/SnpSourceX11.h>
    #include <stream/input/SnpSinkX11Mouse.h>
    #include <stream/input/SnpSinkX11Keyboard.h>
    #include <stream/input/SnpSourceX11Cursor.h>
#endif //HAVE_LIBX11
#ifdef HAVE_LIBWEBSOCKETS
#include <stream/network/SnpSinkNetworkWebsocket.h>
#endif //HAVE_LIBWEBSOCKETS
#include <stream/video/SnpEncoderOpenH264.h>
#include <stream/video/SnpDecoderOpenH264.h>
#include "SnpPipeFactory.h"
#include "util/PropertyUtil.h"
#include "stream/video/SnpEncoderAmfH264.h"
#include "stream/video/SnpDecoderAmfH264.h"
#include "stream/video/SnpSourceDda.h"


std::vector<SnpPipe*> SnpPipeFactory::createPipes(PipeMap& pipeMap, SnpContext* context) {
    std::vector<SnpPipe*> pipes;
    for (const auto &pipe: pipeMap) {
        uint32_t pipeId = pipe.first;
        SnpPipe* snpPipe = createPipe(pipeId, pipe.second, context);
        pipes.push_back(snpPipe);
    }
    return pipes;
}

SnpPipe *SnpPipeFactory::createPipe(uint32_t pipeId, const std::vector<snp::Component *>& components, SnpContext* context) {
    SnpPipeOptions videoPipeOptions = {};
    auto pipe = new SnpPipe(videoPipeOptions, pipeId, context);
    for (const auto &component: components) {
        SnpComponent* snpComponent = nullptr;
        switch(component->componenttype()) {
            case snp::COMPONENT_CAPTURE_VIDEO_DUMMY: {
                SnpSourceDummyOptions options;
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                options.fps = PropertyUtil::getPropertyDouble(component, "fps", 30.0);
                options.boxCount = PropertyUtil::getPropertyUint(component, "boxCount", 3);
                options.boxSpeed = PropertyUtil::getPropertyUint(component, "boxSpeed", 1);
                snpComponent = new SnpSourceDummy(options);
            } break;
            case snp::COMPONENT_CAPTURE_DDA: {
                SnpSourceDdaOptions options;
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                snpComponent = new SnpSourceDda(options);
            } break;
            case snp::COMPONENT_ENCODER_OPENH264: {
                SnpEncoderOpenH264Options options;
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                options.fps = PropertyUtil::getPropertyDouble(component, "fps", 20.0);
                options.qp = PropertyUtil::getPropertyUint(component, "qp", 30);
                snpComponent = new SnpEncoderOpenH264(options);
            } break;
            case snp::COMPONENT_DECODER_OPENH264: {
                SnpDecoderOpenH264Options options = {};
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                options.qp = PropertyUtil::getPropertyUint(component, "qp", 30);
                snpComponent = new SnpDecoderOpenH264(options);
            } break;
            case snp::COMPONENT_OUTPUT_VIDEO_DISPLAY: {
                SnpSinkDisplayOptions options = {};
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                snpComponent = new SnpSinkDisplay(options);
            } break;
            case snp::COMPONENT_ENCODER_AMD: {
                SnpEncoderAmfH264Options options = {};
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                options.fps = PropertyUtil::getPropertyDouble(component, "fps", 30.0);
                snpComponent = new SnpEncoderAmfH264(options);
            } break;
            case snp::COMPONENT_DECODER_AMD: {
                SnpDecoderAmfH264Options options = {};
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                snpComponent = new SnpDecoderAmfH264(options);
            } break;
            #ifdef HAVE_LIBVA
            case snp::COMPONENT_ENCODER_INTEL: {
                SnpEncoderVaH264Options options = {};
                options.width = PropertyUtil::getPropertyUint(component, "width", 1920);
                options.height = PropertyUtil::getPropertyUint(component, "height", 1080);
                snpComponent = new SnpEncoderVaH264(options);
            } break;
            #endif //HAVE_LIBVA
            case snp::COMPONENT_DECODER_INTEL:
            case snp::COMPONENT_CAPTURE_VIDEO_DRM:
            case snp::COMPONENT_CAPTURE_VIDEO_X11:
            case snp::COMPONENT_CAPTURE_VIDEO_WAYLAND:
            case snp::COMPONENT_CAPTURE_VIDEO_V4L:
            case snp::COMPONENT_INPUT_KEYBOARD_X11:
            case snp::COMPONENT_INPUT_POINTER_X11:
            case snp::COMPONENT_INPUT_CURSOR_X11:
            case snp::COMPONENT_OUTPUT_KEYBOARD_X11:
            case snp::COMPONENT_OUTPUT_POINTER_X11:
            case snp::COMPONENT_OUTPUT_CURSOR_X11:
            case snp::ComponentType_INT_MIN_SENTINEL_DO_NOT_USE_:
            case snp::ComponentType_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        snpComponent->setPipeId(pipeId);
        pipe->addComponentEnd(snpComponent);
    }
    return pipe;
}

