#ifndef SNPSERVER_SNPENCODERMMALH264_H
#define SNPSERVER_SNPENCODERMMALH264_H

#include <stream/SnpComponent.h>
#include <string>

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_types.h"
#include "interface/mmal/vc/mmal_vc_shm.h"
#include "interface/vcsm/user-vcsm.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/core/mmal_port_private.h"


struct SnpEncoderMmalH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t qp;
};

class SnpEncoderMmalH264 : public SnpComponent {
public:
    explicit SnpEncoderMmalH264(const SnpEncoderMmalH264Options &options);
    ~SnpEncoderMmalH264() override;

    bool start() override;
    void stop() override;

private:
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    VCOS_SEMAPHORE_T semaphore;
    MMAL_QUEUE_T *queue = nullptr;
    MMAL_COMPONENT_T *encoder = nullptr;
    MMAL_POOL_T *pool_in = nullptr;
    MMAL_POOL_T *pool_out = nullptr;

    void onInputData(uint32_t pipeId, const uint8_t *data, int len, bool complete);

    bool mmalEncoderInit();
    bool mmalEncoderEncode();
    void mmalOnFrameCallback(MMAL_BUFFER_HEADER_T *bufferHeader);
    void mmalEncoderCleanup();
    bool mmalEncoderDestroy();

    static uint8_t *mmalDmaBufAllocator(MMAL_PORT_T *port, uint32_t payload_size);

    static void mmalControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    static void mmalInputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    static void mmalOutputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
};

#endif //SNPSERVER_SNPENCODERMMALH264_H
