#ifndef SNPSERVER_SNPENCODERVAH264_H
#define SNPSERVER_SNPENCODERVAH264_H

#include <cstdint>
#include <stream/video/h264/VaBitstream.h>
#include <d3d12.h>
#include <wrl.h>
#include <fstream>
#include <dxgi1_4.h>
#include "stream/SnpComponent.h"
#include "va/va.h"

using Microsoft::WRL::ComPtr;

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

    //pipeline objects
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    UINT m_rtvDescriptorSize = 0;

    //va objects
    VADisplay vaDisplay;
    VASurfaceID vaRenderTargets[FrameCount] = { };
    static const uint32_t vaNumRGBASurfaces = 16;
    VASurfaceID vaRGBASurfaces[vaNumRGBASurfaces] = { };
    VASurfaceID vaSurfaceNV12 = 0;
    UINT numVPRegions;
    VAConfigID vaProcConfigId = 0;
    // Context for color rgb to yuv conversion
    VAContextID vaColorConvCtx = 0;
    VABufferID vaColorConvBuf = 0;
    VAContextID vaCopyCtx = 0;
    VABufferID vaCopyBuf = 0;
    VAContextID vaBlendCtx = 0;
    VABufferID vaBlendBuf = 0;
    VAProcPipelineCaps procPipelineCaps = { };
    const float alphaBlend = 0.75f;
    const float regionsSizeRatio = 1.2f;
    static const UINT regionVariations = 216;
    UINT curRegionVariation = 0;
    VARectangle pBlendRegions[regionVariations/*Prepare two sets of regions so there's some motion*/][vaNumRGBASurfaces];
    float colors[regionVariations][4];

    // Video Encoder
    VAConfigID vaEncConfigId = 0;
    VAContextID vaEncContextId = 0;
    VABufferID vaEncPipelineBufferId[4];
    static const VABufferID VA_H264ENC_BUFFER_INDEX_SEQ = 0;
    static const VABufferID VA_H264ENC_BUFFER_INDEX_PIC = 1;
    static const VABufferID VA_H264ENC_BUFFER_INDEX_SLICE = 2;
    static const VABufferID VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT = 3;
    static const UINT H264_MB_PIXEL_SIZE = 16;
    std::ofstream finalEncodedBitstream;

    static void vaInfoCallback(void *context, char *message);
    static void vaErrorCallback(void *context, char *message);

    bool initVaPipeline();
    bool initD3D12Pipeline();

    bool initVaDisplay();
    bool ensureVaProcSupport();
    bool ensureVaEncSupport();
    bool createVaSurfaces();
    bool importRenderTargetsToVa();
    bool initVaProcContext();
    bool initVaEncoder();
    bool initVaEncContext();

    bool performVaEncodeFrame(VASurfaceID dstSurface, VABufferID dstCompressedBit);
};

#endif //SNPSERVER_SNPENCODERVAH264_H
