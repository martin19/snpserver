#ifndef SNPSERVER_SNPENCODERVAH264_H
#define SNPSERVER_SNPENCODERVAH264_H

#include <cstdint>
#include <stream/video/h264/VaBitstream.h>
#include <d3d12.h>
#include "stream/SnpComponent.h"
#include "va/va.h"

#define SURFACE_NUM 1
#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)>(b)?(a):(b))

struct SnpEncoderVaH264Options : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
    uint32_t qp;
};

class SnpEncoderVaH264 : public SnpComponent {
public:
    SnpEncoderVaH264(const SnpEncoderVaH264Options &options);
    ~SnpEncoderVaH264() override;

    bool start() override;
    void stop() override;

private:
    uint32_t width;
    uint32_t height;

    //taken roughly from h264 encoder example from libva utils
    bool VaH264EncoderInit();

    void onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete);

    static const UINT FrameCount = 2;
//    ComPtr<ID3D12Device> device;
//    ComPtr<ID3D12Resource> renderTargets[FrameCount];

    VADisplay vaDisplay;
    static const uint32_t vaNumRGBASurfaces = 16;
    VASurfaceID vaRGBASurfaces[vaNumRGBASurfaces] = { };
    VASurfaceID vaSurfaceNV12 = 0;

    bool initVaEncoder();

    static void vaInfoCallback(void *context, char *message);
    static void vaErrorCallback(void *context, char *message);

    bool initVaPipeline();
    bool initVaDisplay();
    bool ensureVaProcSupport();
    bool ensureVaEncSupport();
    bool createVaSurfaces();
    bool importRenderTargetsToVa();
};

#endif //SNPSERVER_SNPENCODERVAH264_H
