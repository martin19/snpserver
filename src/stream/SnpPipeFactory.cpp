#include "util/loguru.h"
#include "SnpPipeFactory.h"
#include "SnpPipe.h"
#include "stream/video/SnpSourceDummy.h"

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



SnpPipe *SnpPipeFactory::createPipe(uint32_t streamId,
                                    SnpComponent *source,
                                    SnpComponent *sink,
                                    snappyv1::StreamMedium medium,
                                    snappyv1::StreamDirection direction,
                                    snappyv1::StreamEndpoint endpoint,
                                    snappyv1::StreamEncoding encoding) {
    switch(medium) {
        case snappyv1::STREAM_MEDIUM_VIDEO:
            return createVideoPipe(streamId, source, sink, direction, endpoint, encoding);
        case snappyv1::STREAM_MEDIUM_AUDIO:
            return createAudioPipe(streamId, source, sink, direction, endpoint, encoding);
        case snappyv1::STREAM_MEDIUM_PERIPHERAL:
            return createPeripheralPipe(streamId, source, sink, direction, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createVideoPipe(uint32_t streamId,
                                         SnpComponent *source,
                                         SnpComponent *sink,
                                         snappyv1::StreamDirection direction,
                                         snappyv1::StreamEndpoint endpoint,
                                         snappyv1::StreamEncoding encoding) {
    switch(direction) {
        case snappyv1::STREAM_DIRECTION_INPUT:
            return createVideoInputPipe(streamId, source, endpoint, encoding);
        case snappyv1::STREAM_DIRECTION_OUTPUT:
            return createVideoOutputPipe(streamId, sink, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createAudioPipe(uint32_t streamId,
                                         SnpComponent *source,
                                         SnpComponent *sink,
                                         snappyv1::StreamDirection direction,
                                         snappyv1::StreamEndpoint endpoint,
                                         snappyv1::StreamEncoding encoding) {
    switch(direction) {
        case snappyv1::STREAM_DIRECTION_INPUT:
            return createAudioInputPipe(streamId, source, endpoint, encoding);
        case snappyv1::STREAM_DIRECTION_OUTPUT:
            return createAudioOutputPipe(streamId, sink, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createPeripheralPipe(uint32_t streamId,
                                              SnpComponent *source,
                                              SnpComponent *sink,
                                              snappyv1::StreamDirection direction,
                                              snappyv1::StreamEndpoint endpoint,
                                              snappyv1::StreamEncoding encoding) {
    switch(direction) {
        case snappyv1::STREAM_DIRECTION_INPUT:
            return createPeripheralInputPipe(streamId, source, endpoint, encoding);
        case snappyv1::STREAM_DIRECTION_OUTPUT:
            return createPeripheralOutputPipe(streamId, sink, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createVideoInputPipe(uint32_t streamId,
                                              SnpComponent *source,
                                              snappyv1::StreamEndpoint endpoint,
                                              snappyv1::StreamEncoding encoding) {

    SnpComponent *decoder = nullptr;
    SnpComponent *sink = nullptr;

    if(endpoint == snappyv1::STREAM_ENDPOINT_DISPLAY) {
        //TODO: create a display component
    } else {
        LOG_F(WARNING, "endpoint (%d) unavailable.", endpoint);
    }

    if(encoding == snappyv1::STREAM_ENCODING_H264_OPENH264) {
        SnpDecoderOpenH264Options decoderOptions = {};
        decoder = new SnpDecoderOpenH264(decoderOptions);
    } else {
        LOG_F(WARNING, "encoding (%d) unavailable.", encoding);
    }

    SnpPort::connect(source->getOutputPort(0), decoder->getInputPort(0));
    SnpPort::connect(decoder->getOutputPort(0), sink->getInputPort(0));

    SnpPipeOptions videoPipeOptions = {};
    auto pipe = new SnpPipe(videoPipeOptions);
    pipe->addComponent(source);
    pipe->addComponent(decoder);
    if(sink != nullptr) pipe->addComponent(sink);

    //TODO: set actual values!!
    pipe->setMedium(snappyv1::STREAM_MEDIUM_VIDEO);
    pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_DISPLAY);
    pipe->setEncoding(snappyv1::STREAM_ENCODING_H264_OPENH264);
    pipe->setDirection(snappyv1::STREAM_DIRECTION_OUTPUT);

    return pipe;
}

SnpPipe *SnpPipeFactory::createVideoOutputPipe(uint32_t streamId,
                                               SnpComponent *sink,
                                               snappyv1::StreamEndpoint endpoint,
                                               snappyv1::StreamEncoding encoding) {
    SnpComponent *source = nullptr;
    SnpComponent *encoder = nullptr;
    LOG_F(INFO, "creating video pipe.");

    if(endpoint == snappyv1::STREAM_ENDPOINT_DRM) {
#ifdef HAVE_LIBDRM
        SnpSourceGLOptions sourceOptions;
        sourceOptions.device = "/dev/dri/card0";
        sourceOptions.fps = 30;
        source = new SnpSourceGL(sourceOptions);
#else
        source = nullptr;
        LOG_F(WARNING, "x11 endpoint (%d) unavailable.", endpoint);
#endif
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_X11) {
#ifdef HAVE_LIBDRM
        SnpSourceX11Options sourceOptions;
        sourceOptions.display = ":0.0";
        source = new SnpSourceX11(sourceOptions);
#else
        source = nullptr;
        LOG_F(WARNING, "x11 endpoint (%d) unavailable.", endpoint);
#endif
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_VIDEO_DUMMY) {
        SnpSourceDummyOptions sourceOptions;
        sourceOptions.width = 1920;
        sourceOptions.height = 1080;
        sourceOptions.fps = 30.0;
        source = new SnpSourceDummy(sourceOptions);
    }
    else {
        LOG_F(WARNING, "unknown endpoint (%d) requested.", endpoint);
        return nullptr;
    }

//TODO: raspberry MMAL
//        SnpEncoderMmalH264Options encoderMmalH264Options = {};
//        encoderMmalH264Options.qp = 20;
//        auto *encoderMmalH264 = new SnpEncoderMmalH264(encoderMmalH264Options);

    if(encoding == snappyv1::STREAM_ENCODING_H264_OPENH264) {
        //OPENH264
        SnpEncoderOpenH264Options encoderOptions = {};
        encoder = new SnpEncoderOpenH264(encoderOptions);
    } else if(encoding == snappyv1::STREAM_ENCODING_H264_HARDWARE) {
#ifdef HAVE_LIBVA
        SnpEncoderVaH264Options encoderOptions = {};
        encoder = new SnpEncoderVaH264(encoderOptions);
#else
        LOG_F(WARNING, "va encoder (%d) not available.", encoding);
#endif //HAVE_LIBVA
    } else {
        LOG_F(WARNING, "unknown encoding (%d) requested.", encoding);
        return nullptr;
    }

    if(encoder == nullptr) {
        LOG_F(ERROR, "cannot instantiate requested encoder component for pipe.");
        return nullptr;
    }

    SnpPort::connect(source->getOutputPort(0), encoder->getInputPort(0));
    SnpPort::connect(encoder->getOutputPort(0), sink->getInputPort(0));

    SnpPipeOptions videoPipeOptions = {};
    auto pipe = new SnpPipe(videoPipeOptions);
    pipe->addComponent(source);
    pipe->addComponent(encoder);
    if(sink != nullptr) pipe->addComponent(sink);

    //TODO: set actual values!!
    pipe->setMedium(snappyv1::STREAM_MEDIUM_VIDEO);
    pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_VIDEO_DUMMY);
    pipe->setEncoding(snappyv1::STREAM_ENCODING_H264_OPENH264);
    pipe->setDirection(snappyv1::STREAM_DIRECTION_OUTPUT);

    return pipe;
}

SnpPipe *SnpPipeFactory::createAudioInputPipe(uint32_t streamId,
                                              SnpComponent *source,
                                              snappyv1::StreamEndpoint endpoint,
                                              snappyv1::StreamEncoding encoding) {
    return nullptr;
}

SnpPipe *SnpPipeFactory::createAudioOutputPipe(uint32_t streamId,
                                               SnpComponent *sink,
                                               snappyv1::StreamEndpoint endpoint,
                                               snappyv1::StreamEncoding encoding) {
    return nullptr;
}

SnpPipe *SnpPipeFactory::createPeripheralInputPipe(uint32_t streamId,
                                                   SnpComponent *source,
                                                   snappyv1::StreamEndpoint endpoint,
                                                   snappyv1::StreamEncoding encoding) {
    if(endpoint == snappyv1::STREAM_ENDPOINT_POINTER) {
#ifdef HAVE_LIBX11
        LOG_F(INFO, "creating mouse pipe.");
        SnpSourceNetworkOptions sourceNetworkOptions = {};
        sourceNetworkOptions.client = client;
        sourceNetworkOptions.streamId = streamId;
        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
        SnpSinkX11MouseOptions sinkMouseOptions = {};
        sinkMouseOptions.width = 1920;
        sinkMouseOptions.height = 1080;
        auto *snpSinkMouse = new SnpSinkX11Mouse(sinkMouseOptions);
        SnpPort::connect(sourceNetwork->getOutputPort(0), snpSinkMouse->getInputPort(0));

        SnpPipeOptions mousePipeOptions = {};
        auto pipe = new SnpPipe(mousePipeOptions);
        pipe->addComponent(sourceNetwork);
        pipe->addComponent(snpSinkMouse);

        pipe->setMedium(snappyv1::STREAM_MEDIUM_PERIPHERAL);
        pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_POINTER);
        pipe->setDirection(snappyv1::STREAM_DIRECTION_INPUT);

        return pipe;
#else
        LOG_F(WARNING, "X11 pointer is unavailable.");
#endif //HAVE_LIBX11
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_KEYBOARD) {
#ifdef HAVE_LIBX11
        LOG_F(INFO, "creating keyboard pipe.");
        SnpSourceNetworkOptions sourceNetworkOptions = {};
        sourceNetworkOptions.client = client;
        sourceNetworkOptions.streamId = streamId;
        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
        SnpSinkX11KeyboardOptions sinkKeyboardOptions = {};
        auto *sinkKeyboard = new SnpSinkX11Keyboard(sinkKeyboardOptions);
        SnpPort::connect(sourceNetwork->getOutputPort(0), sinkKeyboard->getInputPort(0));

        SnpPipeOptions mousePipeOptions = {};
        auto pipe = new SnpPipe(mousePipeOptions);
        pipe->addComponent(sourceNetwork);
        pipe->addComponent(sinkKeyboard);

        pipe->setMedium(snappyv1::STREAM_MEDIUM_PERIPHERAL);
        pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_KEYBOARD);
        pipe->setDirection(snappyv1::STREAM_DIRECTION_INPUT);

        return pipe;
#else
        LOG_F(WARNING, "X11 keyboard is unavailable.");
#endif //HAVE_LIBX11
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_CURSOR) {
#ifdef HAVE_LIBX11
        LOG_F(INFO, "creating cursor pipe.");
        SnpSourceCursorOptions sourceCursorOptions = {};
        auto *sourceCursor = new SnpSourceX11Cursor(sourceCursorOptions);
        SnpSinkNetworkOptions sinkNetworkOptions = {};
        sinkNetworkOptions.client = client;
        sinkNetworkOptions.streamId = streamId;
        auto *sinkNetwork = new SnpSinkNetwork(sinkNetworkOptions);
        SnpPort::connect(sourceCursor->getOutputPort(0), sinkNetwork->getInputPort(0));

        SnpPipeOptions cursorPipeOptions = {};
        auto pipe = new SnpPipe(cursorPipeOptions);
        pipe->addComponent(sourceCursor);
        pipe->addComponent(sinkNetwork);

        pipe->setMedium(snappyv1::STREAM_MEDIUM_PERIPHERAL);
        pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_CURSOR);
        pipe->setDirection(snappyv1::STREAM_DIRECTION_OUTPUT);

        return pipe;
#else
        LOG_F(WARNING, "X11 cursor is unavailable.");
#endif //HAVE_LIBX11
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createPeripheralOutputPipe(uint32_t streamId,
                                                    SnpComponent *sink,
                                                    snappyv1::StreamEndpoint endpoint,
                                                    snappyv1::StreamEncoding encoding) {
    return nullptr;
}