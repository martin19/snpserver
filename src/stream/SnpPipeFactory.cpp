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
#include <stream/network/SnpSinkNetworkTcp.h>
#include <stream/network/SnpSourceNetworkTcp.h>
#include <stream/video/SnpEncoderOpenH264.h>
#include <stream/video/SnpDecoderOpenH264.h>
//
//SnpPipe *SnpPipeFactory::createVideoInputPipe(uint32_t streamId,
//                                              SnpComponent *source,
//                                              snp::StreamEndpoint endpoint,
//                                              snp::StreamEncoding encoding) {
//
//    SnpComponent *decoder = nullptr;
//    SnpComponent *sink = nullptr;
//
//    if(endpoint == snp::COMPONENT_OUTPUT_VIDEO_DISPLAY) {
//        SnpSinkDisplayOptions sinkDisplayOptions = {};
//        sinkDisplayOptions.streamId = 1;
//        sinkDisplayOptions.width = 1920;
//        sinkDisplayOptions.height = 1080;
//        sink = (SnpComponent*)new SnpSinkDisplay(sinkDisplayOptions);
//    } else {
//        LOG_F(WARNING, "display endpoint (%d) unavailable.", endpoint);
//    }
//
//    if(encoding == snp::COMPONENT_ENCODER_OPENH264) {
//        SnpDecoderOpenH264Options decoderOptions = {};
//        decoder = new SnpDecoderOpenH264(decoderOptions);
//    } else {
//        LOG_F(WARNING, "encoding (%d) unavailable.", encoding);
//    }
//
//    SnpPort::connect(source->getOutputPort(0), decoder->getInputPort(0));
//    SnpPort::connect(decoder->getOutputPort(0), sink->getInputPort(0));
//
//    SnpPipeOptions videoPipeOptions = {};
//    auto pipe = new SnpPipe(videoPipeOptions);
//    pipe->addComponent(source);
//    pipe->addComponent(decoder);
//    if(sink != nullptr) pipe->addComponent(sink);
//
//    //TODO: set actual values!!
//    pipe->setMedium(snp::STREAM_MEDIUM_VIDEO);
//    pipe->setEndpoint(snp::STREAM_ENDPOINT_DISPLAY);
//    pipe->setEncoding(snp::STREAM_ENCODING_H264_OPENH264);
//    pipe->setDirection(snp::STREAM_DIRECTION_OUTPUT);
//
//    return pipe;
//}
//
//SnpPipe *SnpPipeFactory::createVideoOutputPipe(uint32_t streamId,
//                                               SnpComponent *sink,
//                                               snp::StreamEndpoint endpoint,
//                                               snp::StreamEncoding encoding) {
//    SnpComponent *source = nullptr;
//    SnpComponent *encoder = nullptr;
//    LOG_F(INFO, "creating video pipe.");
//
//    if(endpoint == snp::COMPONENT_CAPTURE_VIDEO_DRM) {
//#ifdef HAVE_LIBDRM
//        SnpSourceGLOptions sourceOptions;
//        sourceOptions.device = "/dev/dri/card0";
//        sourceOptions.fps = 30;
//        source = new SnpSourceGL(sourceOptions);
//#else
//        source = nullptr;
//        LOG_F(WARNING, "x11 endpoint (%d) unavailable.", endpoint);
//#endif
//    } else if(endpoint == snp::COMPONENT_CAPTURE_VIDEO_X11) {
//#ifdef HAVE_LIBDRM
//        SnpSourceX11Options sourceOptions;
//        sourceOptions.display = ":0.0";
//        source = new SnpSourceX11(sourceOptions);
//#else
//        source = nullptr;
//        LOG_F(WARNING, "x11 endpoint (%d) unavailable.", endpoint);
//#endif
//    } else if(endpoint == snp::COMPONENT_CAPTURE_VIDEO_DUMMY) {
//        SnpSourceDummyOptions sourceOptions;
//        sourceOptions.width = 1920;
//        sourceOptions.height = 1080;
//        sourceOptions.fps = 30.0;
//        source = new SnpSourceDummy(sourceOptions);
//    }
//    else {
//        LOG_F(WARNING, "unknown endpoint (%d) requested.", endpoint);
//        return nullptr;
//    }
//
////TODO: raspberry MMAL
////        SnpEncoderMmalH264Options encoderMmalH264Options = {};
////        encoderMmalH264Options.qp = 20;
////        auto *encoderMmalH264 = new SnpEncoderMmalH264(encoderMmalH264Options);
//
//    if(encoding == snp::COMPONENT_ENCODER_OPENH264) {
//        //OPENH264
//        SnpEncoderOpenH264Options encoderOptions = {};
//        encoder = new SnpEncoderOpenH264(encoderOptions);
//    } else if(encoding == snp::COMPONENT_ENCODER_INTEL) {
//#ifdef HAVE_LIBVA
//        SnpEncoderVaH264Options encoderOptions = {};
//        encoder = new SnpEncoderVaH264(encoderOptions);
//#else
//        LOG_F(WARNING, "va encoder (%d) not available.", encoding);
//#endif //HAVE_LIBVA
//    } else {
//        LOG_F(WARNING, "unknown encoding (%d) requested.", encoding);
//        return nullptr;
//    }
//
//    if(encoder == nullptr) {
//        LOG_F(ERROR, "cannot instantiate requested encoder component for pipe.");
//        return nullptr;
//    }
//
//    SnpPort::connect(source->getOutputPort(0), encoder->getInputPort(0));
//    SnpPort::connect(encoder->getOutputPort(0), sink->getInputPort(0));
//
//    SnpPipeOptions videoPipeOptions = {};
//    auto pipe = new SnpPipe(videoPipeOptions);
//    pipe->addComponent(source);
//    pipe->addComponent(encoder);
//    if(sink != nullptr) pipe->addComponent(sink);
//
//    //TODO: set actual values!!
//    pipe->setMedium(snp::STREAM_MEDIUM_VIDEO);
//    pipe->setEndpoint(snp::STREAM_ENDPOINT_VIDEO_DUMMY);
//    pipe->setEncoding(snp::STREAM_ENCODING_H264_OPENH264);
//    pipe->setDirection(snp::STREAM_DIRECTION_OUTPUT);
//
//    return pipe;
//}
//
//SnpPipe *SnpPipeFactory::createPeripheralInputPipe(uint32_t streamId,
//                                                   SnpComponent *source,
//                                                   snp::StreamEndpoint endpoint,
//                                                   snp::StreamEncoding encoding) {
//    if(endpoint == snp::COMPONENT_INPUT_POINTER_X11) {
//#ifdef HAVE_LIBX11
//        LOG_F(INFO, "creating mouse pipe.");
//        SnpSourceNetworkOptions sourceNetworkOptions = {};
//        sourceNetworkOptions.client = client;
//        sourceNetworkOptions.streamId = streamId;
//        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
//        SnpSinkX11MouseOptions sinkMouseOptions = {};
//        sinkMouseOptions.width = 1920;
//        sinkMouseOptions.height = 1080;
//        auto *snpSinkMouse = new SnpSinkX11Mouse(sinkMouseOptions);
//        SnpPort::connect(sourceNetwork->getOutputPort(0), snpSinkMouse->getInputPort(0));
//
//        SnpPipeOptions mousePipeOptions = {};
//        auto pipe = new SnpPipe(mousePipeOptions);
//        pipe->addComponent(sourceNetwork);
//        pipe->addComponent(snpSinkMouse);
//
//        pipe->setMedium(snp::STREAM_MEDIUM_PERIPHERAL);
//        pipe->setEndpoint(snp::STREAM_ENDPOINT_POINTER);
//        pipe->setDirection(snp::STREAM_DIRECTION_INPUT);
//
//        return pipe;
//#else
//        LOG_F(WARNING, "X11 pointer is unavailable.");
//#endif //HAVE_LIBX11
//    } else if(endpoint == snp::COMPONENT_INPUT_KEYBOARD_X11) {
//#ifdef HAVE_LIBX11
//        LOG_F(INFO, "creating keyboard pipe.");
//        SnpSourceNetworkOptions sourceNetworkOptions = {};
//        sourceNetworkOptions.client = client;
//        sourceNetworkOptions.streamId = streamId;
//        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
//        SnpSinkX11KeyboardOptions sinkKeyboardOptions = {};
//        auto *sinkKeyboard = new SnpSinkX11Keyboard(sinkKeyboardOptions);
//        SnpPort::connect(sourceNetwork->getOutputPort(0), sinkKeyboard->getInputPort(0));
//
//        SnpPipeOptions mousePipeOptions = {};
//        auto pipe = new SnpPipe(mousePipeOptions);
//        pipe->addComponent(sourceNetwork);
//        pipe->addComponent(sinkKeyboard);
//
//        pipe->setMedium(snp::STREAM_MEDIUM_PERIPHERAL);
//        pipe->setEndpoint(snp::STREAM_ENDPOINT_KEYBOARD);
//        pipe->setDirection(snp::STREAM_DIRECTION_INPUT);
//
//        return pipe;
//#else
//        LOG_F(WARNING, "X11 keyboard is unavailable.");
//#endif //HAVE_LIBX11
//    } else if(endpoint == snp::COMPONENT_INPUT_CURSOR_X11) {
//#ifdef HAVE_LIBX11
//        LOG_F(INFO, "creating cursor pipe.");
//        SnpSourceCursorOptions sourceCursorOptions = {};
//        auto *sourceCursor = new SnpSourceX11Cursor(sourceCursorOptions);
//        SnpSinkNetworkOptions sinkNetworkOptions = {};
//        sinkNetworkOptions.client = client;
//        sinkNetworkOptions.streamId = streamId;
//        auto *sinkNetwork = new SnpSinkNetwork(sinkNetworkOptions);
//        SnpPort::connect(sourceCursor->getOutputPort(0), sinkNetwork->getInputPort(0));
//
//        SnpPipeOptions cursorPipeOptions = {};
//        auto pipe = new SnpPipe(cursorPipeOptions);
//        pipe->addComponent(sourceCursor);
//        pipe->addComponent(sinkNetwork);
//
//        pipe->setMedium(snp::STREAM_MEDIUM_PERIPHERAL);
//        pipe->setEndpoint(snp::STREAM_ENDPOINT_CURSOR);
//        pipe->setDirection(snp::STREAM_DIRECTION_OUTPUT);
//
//        return pipe;
//#else
//        LOG_F(WARNING, "X11 cursor is unavailable.");
//#endif //HAVE_LIBX11
//    }
//
//    return nullptr;
//}
//
//SnpPipe *SnpPipeFactory::createPipe(uint32_t streamId, std::vector<SnpComponent *> components) {
//    return nullptr;
//}
#include "SnpPipeFactory.h"

std::vector<SnpPipe*>* SnpPipeFactory::createPipes(SnpConfig *pConfig, std::string side) {
    auto pipes = new std::vector<SnpPipe*>();
    PipeMap* pipeMap = nullptr;
    if(side == "local") {
        pipeMap = pConfig->getLocalPipes();
    } else {
        pipeMap = pConfig->getRemotePipes();
    }
    for (const auto &pipe: *pipeMap) {
        uint32_t pipeId = pipe.first;
        SnpPipe* snpPipe = createPipe(pipeId, &pipe.second);
        pipes->push_back(snpPipe);
    }
    return pipes;
}

SnpPipe *SnpPipeFactory::createPipe(uint32_t pipeId, const std::vector<snp::Component *> *components) {
    SnpPipeOptions videoPipeOptions = {};
    auto pipe = new SnpPipe(videoPipeOptions, pipeId);
    for (const auto &component: *components) {
        SnpComponent* snpComponent = nullptr;
        switch(component->componenttype()) {
            case snp::COMPONENT_CAPTURE_VIDEO_DUMMY: {
                SnpSourceDummyOptions options;
                options.width = SnpConfig::getPropertyUint(component, "width", 1920);
                options.height = SnpConfig::getPropertyUint(component, "height", 1080);
                options.fps = SnpConfig::getPropertyDouble(component, "fps", 30.0);
                snpComponent = new SnpSourceDummy(options);
            } break;
            case snp::COMPONENT_ENCODER_OPENH264: {
                SnpEncoderOpenH264Options options;
                options.width = SnpConfig::getPropertyUint(component, "width", 1920);
                options.height = SnpConfig::getPropertyUint(component, "height", 1080);
                options.qp = SnpConfig::getPropertyUint(component, "qp", 30);
                snpComponent = new SnpEncoderOpenH264(options);
            } break;
            case snp::COMPONENT_DECODER_OPENH264: {
                SnpDecoderOpenH264Options options = {};
                options.width = SnpConfig::getPropertyUint(component, "width", 1920);
                options.height = SnpConfig::getPropertyUint(component, "height", 1080);
                options.qp = SnpConfig::getPropertyUint(component, "qp", 30);
                snpComponent = new SnpDecoderOpenH264(options);
            } break;
            case snp::COMPONENT_OUTPUT_VIDEO_DISPLAY: {
                SnpSinkDisplayOptions options = {};
                options.width = SnpConfig::getPropertyUint(component, "width", 1920);
                options.height = SnpConfig::getPropertyUint(component, "height", 1080);
                snpComponent = new SnpSinkDisplay(options);
            } break;
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
            case snp::COMPONENT_ENCODER_INTEL:
            case snp::COMPONENT_DECODER_INTEL:
            case snp::COMPONENT_ENCODER_AMD:
            case snp::COMPONENT_DECODER_AMD:
            case snp::ComponentType_INT_MIN_SENTINEL_DO_NOT_USE_:
            case snp::ComponentType_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        snpComponent->setPipeId(pipeId);
        pipe->addComponentEnd(snpComponent);
    }
}

