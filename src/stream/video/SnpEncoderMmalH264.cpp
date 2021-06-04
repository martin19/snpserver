#include "SnpEncoderMmalH264.h"
#include "SnpSourceGL.h"
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <iostream>

#define CHECK_STATUS(status, msg) if (status != MMAL_SUCCESS) { fprintf(stderr, msg"\n"); goto error; }

SnpEncoderMmalH264::SnpEncoderMmalH264(const SnpEncoderMmalH264Options &options) : SnpComponent(options) {
    width = options.width;
    height = options.height;
    bpp = options.bpp;

    addInputPort(new SnpPort(PORT_TYPE_BOTH));
    addOutputPort(new SnpPort());

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderMmalH264::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

SnpEncoderMmalH264::~SnpEncoderMmalH264() {
    mmalEncoderDestroy();
}

void SnpEncoderMmalH264::onInputData(const uint8_t *data, int len, bool complete) {
    if(!isEnabled()) return;
    mmalEncoderEncode();
}

void SnpEncoderMmalH264::setEnabled(bool enabled) {
    if(enabled) {
        mmalEncoderInit();
    } else {
        mmalEncoderDestroy();
    }
    SnpComponent::setEnabled(enabled);
}

void SnpEncoderMmalH264::mmalControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    if(buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
        printf("MMAL_EVENT_PARAMETER_CHANGED\n");
    } else if(buffer->cmd == MMAL_EVENT_FORMAT_CHANGED) {
        printf("MMAL_EVENT_FORMAT_CHANGED\n");
    } else if(buffer->cmd == MMAL_EVENT_EOS) {
        printf("MMAL_EVENT_EOS\n");
    } else if(buffer->cmd == MMAL_EVENT_ERROR) {
        printf("MMAL_EVENT_ERROR\n");
        printf("Error type %d", *(MMAL_STATUS_T*)buffer->data);
    }
}

void SnpEncoderMmalH264::mmalInputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    auto *ctx = (SnpEncoderMmalH264 *) port->userdata;
    /* The decoder is done with the data, just recycle the buffer header into its pool */
    mmal_buffer_header_release(buffer);
    /* Kick the processing thread */
    vcos_semaphore_post(&ctx->semaphore);
}

void SnpEncoderMmalH264::mmalOutputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    auto *ctx = (SnpEncoderMmalH264 *) port->userdata;
    /* Queue the decoded video frame */
    mmal_queue_put(ctx->queue, buffer);
    /* Kick the processing thread */
    vcos_semaphore_post(&ctx->semaphore);
}

uint8_t *SnpEncoderMmalH264::mmalDmaBufAllocator(MMAL_PORT_T *port, uint32_t payload_size) {
    unsigned int vcsm_handle = 0;
    unsigned int vc_opaque_handle = 0;
    auto *ctx = (SnpEncoderMmalH264 *) port->userdata;
    SnpPort *inputPort = ctx->getInputPort(0);

    if(vcsm_init() != 0) {
        fprintf(stderr, "Cannot initDrm vcsm (vcsm_init)\n");
        goto error;
    }

    vcsm_handle = vcsm_import_dmabuf(inputPort->dmaBufFd, inputPort->device.c_str());
    if(!vcsm_handle) {
        fprintf(stderr, "Cannot import dmabuf vcsm_import_dmabuf(dmabuf=%d, device=%s)\n", inputPort->dmaBufFd, inputPort->device.c_str());
        goto error;
    }
    printf("vcsm_handle = %d\n", vcsm_handle);

    //https://github.com/6by9/drm_mmal/blob/master/drm_mmal.c
    //TODO: cast?
    vc_opaque_handle = vcsm_vc_hdl_from_hdl(vcsm_handle);
    if(!vc_opaque_handle) {
        fprintf(stderr, "Cannot get opaque handle from vcsm_handle\n");
        goto error;
    }

    return (uint8_t*)vc_opaque_handle;
error:
    vcsm_free(vcsm_handle);
//    vcsm_exit();
    return nullptr;
}

bool SnpEncoderMmalH264::mmalEncoderInit() {
    MMAL_STATUS_T status = MMAL_SUCCESS;
    MMAL_PORT_T *encoder_output = nullptr;
    MMAL_PORT_T *encoder_input = nullptr;
    MMAL_ES_FORMAT_T *format_out = nullptr;
    MMAL_ES_FORMAT_T *format_in = nullptr;

    vcos_semaphore_create(&this->semaphore, "example", 1);

    /* Create the encoder component.
     * This specific component exposes 2 ports (1 input and 1 output). Like most components
     * its expects the format of its input port to be set by the client in order for it to
     * know what kind of data it will be fed. */
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);
    CHECK_STATUS(status, "failed to create decoder");

    /* Set format of video decoder input port */
    format_in = encoder->input[0]->format;
    format_in->type = MMAL_ES_TYPE_VIDEO;
//    format_in->encoding = MMAL_ENCODING_RGB16;
    format_in->encoding = MMAL_ENCODING_BGRA;
    //format_in->encoding = MMAL_ENCODING_RGBA;
    format_in->es->video.width = this->width;
    format_in->es->video.height = this->height;
    format_in->es->video.frame_rate.num = 30;
    format_in->es->video.frame_rate.den = 1;
    format_in->es->video.par.num = 1;
    format_in->es->video.par.den = 1;
    format_in->es->video.crop.x = 0;
    format_in->es->video.crop.y = 0;
    format_in->es->video.crop.width = this->width;
    format_in->es->video.crop.height = this->height;

    /* If the data is known to be framed then the following flag should be set:
     * format_in->flags |= MMAL_ES_FORMAT_FLAG_FRAMED; */

    status = mmal_port_format_commit(encoder->input[0]);
    CHECK_STATUS(status, "failed to commit input port format");

    format_out = encoder->output[0]->format;
    format_out->type = MMAL_ES_TYPE_VIDEO;
    format_out->encoding = MMAL_ENCODING_H264;
    format_out->es->video.width = this->width;
    format_out->es->video.height = this->height;
    format_out->es->video.frame_rate.num = 30;
    format_out->es->video.frame_rate.den = 1;
    format_out->es->video.par.num = 1;
    format_out->es->video.par.den = 1;
    status = mmal_port_format_commit(encoder->output[0]);
    CHECK_STATUS(status, "failed to commit output port format");

    encoder_input = encoder->input[0];
    encoder_output = encoder->output[0];

    //configure zero copy mode on input
    mmal_port_parameter_set_boolean(encoder_input, MMAL_PARAMETER_ZERO_COPY, 1);

    //additional parameters
//    mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_MINIMISE_FRAGMENTATION, 1);
//    mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_H264_DISABLE_CABAC, 1);
//    mmal_port_parameter_set_boolean(encoder_input, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, 1);

    //configure h264 encoding
    //see https://raw.githubusercontent.com/raspberrypi/userland/master/host_applications/linux/apps/raspicam/RaspiVid.c

    {
        MMAL_PARAMETER_VIDEO_PROFILE_T param;
        param.hdr.id = MMAL_PARAMETER_PROFILE;
        param.hdr.size = sizeof(param);
        param.profile[0].profile = MMAL_VIDEO_PROFILE_H264_CONSTRAINED_BASELINE;
        param.profile[0].level = MMAL_VIDEO_LEVEL_H264_42;
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_PROFILE");
    }

    //seems to make image crisper and increases framerate ?
    {
        MMAL_PARAMETER_UINT32_T param = {{MMAL_PARAMETER_VIDEO_ENCODE_H264_DEBLOCK_IDC, sizeof(param)}, 0};
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_VIDEO_ENCODE_H264_DEBLOCK_IDC");
    }

    //quality (quantisationParameter 0..51)
    {
        MMAL_PARAMETER_UINT32_T param = {{MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, 15};
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT");
    }

    {
        MMAL_PARAMETER_UINT32_T param = {{MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, 15};
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT");
    }

    {
        MMAL_PARAMETER_UINT32_T param = {{MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, 15};
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT");
    }

    {
        MMAL_PARAMETER_UINT32_T param = {{MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, 30000};
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_INTRAPERIOD");
    }

    {
        MMAL_PARAMETER_BOOLEAN_T param = {{MMAL_PARAMETER_VIDEO_ENCODE_HEADERS_WITH_FRAME, sizeof(param)}, 0};
        status = mmal_port_parameter_set(encoder_output, &param.hdr);
        CHECK_STATUS(status, "failed to set port parameter MMAL_PARAMETER_VIDEO_ENCODE_HEADERS_WITH_FRAME");
    }

    encoder->input[0]->buffer_num = encoder->input[0]->buffer_num_min;
    encoder->input[0]->buffer_size = encoder->input[0]->buffer_size_recommended;
    encoder->output[0]->buffer_num = encoder->output[0]->buffer_num_min;
    encoder->output[0]->buffer_size = encoder->output[0]->buffer_size_recommended * 5;
    mmal_port_enable(encoder->control, mmalControlCallback);

    /* Store a reference to our context in each port (will be used during callbacks) */
    encoder->input[0]->userdata = ((struct MMAL_PORT_USERDATA_T *) this);
    encoder->output[0]->userdata = ((struct MMAL_PORT_USERDATA_T *) this);

    //custom allocator retreive input data using dmaBuf
    encoder->input[0]->priv->pf_payload_alloc = mmalDmaBufAllocator;

    pool_in = mmal_port_pool_create(encoder->input[0],
                                    encoder->input[0]->buffer_num,
                                    encoder->input[0]->buffer_size);
    pool_out = mmal_pool_create(encoder->output[0]->buffer_num,
                                encoder->output[0]->buffer_size);

    /* Create a queue to store our encoded frames. The callback we will get when
     * a frame has been encoded will put the frame into this queue. */
    queue = mmal_queue_create();

    /* Enable all the input port and the output port.
     * The callback specified here is the function which will be called when the buffer header
     * we sent to the component has been processed. */
    status = mmal_port_enable(encoder->input[0], &SnpEncoderMmalH264::mmalInputCallback);
    CHECK_STATUS(status, "failed to enable input port");
    status = mmal_port_enable(encoder->output[0], &SnpEncoderMmalH264::mmalOutputCallback);
    CHECK_STATUS(status, "failed to enable output port");

    /* Component won't start processing data until it is enabled. */
    status = mmal_component_enable(encoder);
    CHECK_STATUS(status, "failed to enable component");

error:
    if (status != MMAL_SUCCESS) mmalEncoderCleanup();
    return status == MMAL_SUCCESS;
}

void SnpEncoderMmalH264::mmalEncoderCleanup() {/* Cleanup everything */
    if (encoder)
        mmal_component_destroy(encoder);
    if (pool_in)
        mmal_pool_destroy(pool_in);
    if (pool_out)
        mmal_pool_destroy(pool_out);
    if (queue)
        mmal_queue_destroy(queue);

    vcos_semaphore_delete(&semaphore);
}

bool SnpEncoderMmalH264::mmalEncoderEncode() {
    MMAL_STATUS_T status = MMAL_SUCCESS;
    MMAL_BUFFER_HEADER_T *bufferHeader;

    /* Wait for bufferHeader headers to be available on either of the encoder ports */
    vcos_semaphore_wait(&semaphore);

    /* Send data to decode to the input port of the video encoder */
    if ((bufferHeader = mmal_queue_get(pool_in->queue)) != nullptr) {
//        dynamic_cast<SnpSourceGL*>(getInputPort(0)->getOwner())->captureFrame();
        bufferHeader->length = width * height * 3; //why 3!?
        bufferHeader->offset = 0;
        bufferHeader->pts = bufferHeader->dts = MMAL_TIME_UNKNOWN;
        bufferHeader->flags = MMAL_BUFFER_HEADER_FLAG_EOS;
//        client->rfbStatistics.encode_ts_start_ms = (uint32_t)(getCaptureTimeNs()/1000000);
        status = mmal_port_send_buffer(encoder->input[0], bufferHeader);
        CHECK_STATUS(status, "failed to send bufferHeader\n");
    }

    /* Get our encoded frames */
    while ((bufferHeader = mmal_queue_get(queue)) != nullptr) {
        mmalOnFrameCallback(bufferHeader);
        mmal_buffer_header_release(bufferHeader);
    }

    /* Send empty buffers to the output port of the decoder */
    while ((bufferHeader = mmal_queue_get(pool_out->queue)) != nullptr) {
        status = mmal_port_send_buffer(encoder->output[0], bufferHeader);
        CHECK_STATUS(status, "failed to send bufferHeader");
    }
error:
    if (status != MMAL_SUCCESS) mmalEncoderCleanup();
    return status == MMAL_SUCCESS;
}

void SnpEncoderMmalH264::mmalOnFrameCallback(MMAL_BUFFER_HEADER_T *bufferHeader) {
    SnpPort *outputPort = this->getOutputPort(0);
    if(bufferHeader->flags & MMAL_BUFFER_HEADER_FLAG_NAL_END) {
        outputPort->onData(bufferHeader->data, bufferHeader->length, true);
    } else {
        outputPort->onData(bufferHeader->data, bufferHeader->length, false);
    }
}

bool SnpEncoderMmalH264::mmalEncoderDestroy() {
    MMAL_STATUS_T status = MMAL_SUCCESS;

    /* Stop everything. Not strictly necessary since mmal_component_destroy()
     * will do that anyway */
    mmal_port_disable(encoder->input[0]);
    mmal_port_disable(encoder->output[0]);
    mmal_component_disable(encoder);

error:
    mmalEncoderCleanup();
    return status == MMAL_SUCCESS;
}
