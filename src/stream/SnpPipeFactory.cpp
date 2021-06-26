#include "util/loguru.h"
#include "SnpPipeFactory.h"

#include <stream/video/SnpSourceGL.h>
#include <stream/video/SnpEncoderVaH264.h>
#include <stream/network/SnpSinkNetwork.h>
#include <stream/video/SnpEncoderOpenH264.h>
#include <stream/network/SnpSourceNetwork.h>
#include <stream/input/SnpSinkMouse.h>
#include <stream/input/SnpSinkKeyboard.h>
#include <stream/input/SnpSourceCursor.h>
#include <stream/video/SnpSourceX11.h>

SnpPipe *SnpPipeFactory::createPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamMedium medium, snappyv1::StreamDirection direction,
                                    snappyv1::StreamEndpoint endpoint, snappyv1::StreamEncoding encoding) {

    switch(medium) {
        case snappyv1::STREAM_MEDIUM_VIDEO:
            return createVideoPipe(streamId, client, direction, endpoint, encoding);
        case snappyv1::STREAM_MEDIUM_AUDIO:
            return createAudioPipe(streamId, client, direction, endpoint, encoding);
        case snappyv1::STREAM_MEDIUM_PERIPHERIAL:
            return createPeripherialPipe(streamId, client, direction, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createVideoPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamDirection direction,
                                         snappyv1::StreamEndpoint endpoint, snappyv1::StreamEncoding encoding) {
    switch(direction) {
        case snappyv1::STREAM_DIRECTION_INPUT:
            return createVideoInputPipe(streamId, client, endpoint, encoding);
        case snappyv1::STREAM_DIRECTION_OUTPUT:
            return createVideoOutputPipe(streamId, client, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createAudioPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamDirection direction,
                                         snappyv1::StreamEndpoint endpoint, snappyv1::StreamEncoding encoding) {
    switch(direction) {
        case snappyv1::STREAM_DIRECTION_INPUT:
            return createAudioInputPipe(streamId, client, endpoint, encoding);
        case snappyv1::STREAM_DIRECTION_OUTPUT:
            return createAudioOutputPipe(streamId, client, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createPeripherialPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamDirection direction,
                                               snappyv1::StreamEndpoint endpoint, snappyv1::StreamEncoding encoding) {
    switch(direction) {
        case snappyv1::STREAM_DIRECTION_INPUT:
            return createPeripherialInputPipe(streamId, client, endpoint, encoding);
        case snappyv1::STREAM_DIRECTION_OUTPUT:
            return createPeripherialOutputPipe(streamId, client, endpoint, encoding);
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createVideoInputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                              snappyv1::StreamEncoding encoding) {
    return nullptr;
}

SnpPipe *SnpPipeFactory::createVideoOutputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                               snappyv1::StreamEncoding encoding) {
    SnpComponent *source = nullptr;
    SnpComponent *encoder = nullptr;
    LOG_F(INFO, "creating video pipe.");

    if(endpoint == snappyv1::STREAM_ENDPOINT_DRM) {
        SnpSourceGLOptions sourceOptions;
        sourceOptions.device = "/dev/dri/card0";
        sourceOptions.fps = 30;
        source = new SnpSourceGL(sourceOptions);
        ///
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_X11) {
        SnpSourceX11Options sourceOptions;
        sourceOptions.display = ":0.0";
        source = new SnpSourceX11(sourceOptions);
    } else {
        LOG_F(WARNING, "unknown endpoint (%d) requested.", endpoint);
        return nullptr;
    }

//TODO: raspberry MMAL
//        SnpEncoderMmalH264Options encoderMmalH264Options = {};
//        encoderMmalH264Options.qp = 20;
//        auto *encoderMmalH264 = new SnpEncoderMmalH264(encoderMmalH264Options);

    if(encoding == snappyv1::STREAM_ENCODING_H264_SOFTWARE) {
        //OPENH264
        SnpEncoderOpenH264Options encoderOptions = {};
        encoder = new SnpEncoderOpenH264(encoderOptions);
    } else if(encoding == snappyv1::STREAM_ENCODING_H264_HARDWARE) {
        //libva
        SnpEncoderVaH264Options encoderOptions = {};
        encoder = new SnpEncoderVaH264(encoderOptions);
    } else {
        LOG_F(WARNING, "unknown encoding (%d) requested.", encoding);
        return nullptr;
    }

    ///
    SnpSinkNetworkOptions sinkOptions = {};
    sinkOptions.client = client;
    sinkOptions.streamId = streamId;
    auto *sink = new SnpSinkNetwork(sinkOptions);

    SnpPort::connect(source->getOutputPort(0), encoder->getInputPort(0));
    SnpPort::connect(encoder->getOutputPort(0), sink->getInputPort(0));

    SnpPipeOptions videoPipeOptions = {};
    auto pipe = new SnpPipe(videoPipeOptions);
    pipe->addComponent(source);
    pipe->addComponent(encoder);
    pipe->addComponent(sink);

    //TODO: set actual values!!
    pipe->setMedium(snappyv1::STREAM_MEDIUM_VIDEO);
    pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_X11);
    pipe->setEncoding(snappyv1::STREAM_ENCODING_H264_HARDWARE);
    pipe->setDirection(snappyv1::STREAM_DIRECTION_OUTPUT);

    return pipe;
}

SnpPipe *SnpPipeFactory::createAudioInputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                              snappyv1::StreamEncoding encoding) {
    return nullptr;
}

SnpPipe *SnpPipeFactory::createAudioOutputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                               snappyv1::StreamEncoding encoding) {
    return nullptr;
}

SnpPipe *SnpPipeFactory::createPeripherialInputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                                    snappyv1::StreamEncoding encoding) {
    if(endpoint == snappyv1::STREAM_ENDPOINT_POINTER) {
        LOG_F(INFO, "creating mouse pipe.");
        SnpSourceNetworkOptions sourceNetworkOptions = {};
        sourceNetworkOptions.client = client;
        sourceNetworkOptions.streamId = streamId;
        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
        SnpSinkMouseOptions sinkMouseOptions = {};
        sinkMouseOptions.width = 1920;
        sinkMouseOptions.height = 1080;
        auto *snpSinkMouse = new SnpSinkMouse(sinkMouseOptions);
        SnpPort::connect(sourceNetwork->getOutputPort(0), snpSinkMouse->getInputPort(0));

        SnpPipeOptions mousePipeOptions = {};
        auto pipe = new SnpPipe(mousePipeOptions);
        pipe->addComponent(sourceNetwork);
        pipe->addComponent(snpSinkMouse);

        pipe->setMedium(snappyv1::STREAM_MEDIUM_PERIPHERIAL);
        pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_POINTER);
        pipe->setDirection(snappyv1::STREAM_DIRECTION_INPUT);

        return pipe;
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_KEYBOARD) {
        LOG_F(INFO, "creating keyboard pipe.");
        SnpSourceNetworkOptions sourceNetworkOptions = {};
        sourceNetworkOptions.client = client;
        sourceNetworkOptions.streamId = streamId;
        auto *sourceNetwork = new SnpSourceNetwork(sourceNetworkOptions);
        SnpSinkKeyboardOptions sinkKeyboardOptions = {};
        auto *sinkKeyboard = new SnpSinkKeyboard(sinkKeyboardOptions);
        SnpPort::connect(sourceNetwork->getOutputPort(0), sinkKeyboard->getInputPort(0));

        SnpPipeOptions mousePipeOptions = {};
        auto pipe = new SnpPipe(mousePipeOptions);
        pipe->addComponent(sourceNetwork);
        pipe->addComponent(sinkKeyboard);

        pipe->setMedium(snappyv1::STREAM_MEDIUM_PERIPHERIAL);
        pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_KEYBOARD);
        pipe->setDirection(snappyv1::STREAM_DIRECTION_INPUT);

        return pipe;
    } else if(endpoint == snappyv1::STREAM_ENDPOINT_CURSOR) {
        LOG_F(INFO, "creating cursor pipe.");
        SnpSourceCursorOptions sourceCursorOptions = {};
        auto *sourceCursor = new SnpSourceCursor(sourceCursorOptions);
        SnpSinkNetworkOptions sinkNetworkOptions = {};
        sinkNetworkOptions.client = client;
        sinkNetworkOptions.streamId = streamId;
        auto *sinkNetwork = new SnpSinkNetwork(sinkNetworkOptions);
        SnpPort::connect(sourceCursor->getOutputPort(0), sinkNetwork->getInputPort(0));

        SnpPipeOptions cursorPipeOptions = {};
        auto pipe = new SnpPipe(cursorPipeOptions);
        pipe->addComponent(sourceCursor);
        pipe->addComponent(sinkNetwork);

        pipe->setMedium(snappyv1::STREAM_MEDIUM_PERIPHERIAL);
        pipe->setEndpoint(snappyv1::STREAM_ENDPOINT_CURSOR);
        pipe->setDirection(snappyv1::STREAM_DIRECTION_OUTPUT);

        return pipe;
    }

    return nullptr;
}

SnpPipe *SnpPipeFactory::createPeripherialOutputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                                     snappyv1::StreamEncoding encoding) {
    return nullptr;
}