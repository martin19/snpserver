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
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t* yuvBuffer = nullptr;

    void onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete);

    static const UINT FrameCount = 2;

    // Synchronization objects.
    UINT frameIndex = 0;
    HANDLE fenceEvent = nullptr;
    ComPtr<ID3D12Fence> fence = nullptr;
    UINT64 fenceValue = 0;

    //pipeline objects
    ComPtr<IDXGISwapChain3> swapChain = nullptr;
    ComPtr<IDXGIAdapter1> adapter = nullptr;
    ComPtr<ID3D12Device> device = nullptr;
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
    ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
    UINT m_rtvDescriptorSize = 0;

    //va objects
    VADisplay vaDisplay = nullptr;
    VASurfaceID vaRenderTargets[FrameCount] = { };
    VASurfaceID vaSurfaceYuv = 0;

    // Video Encoder
    VAConfigID vaEncConfigId = 0;
    VAContextID vaEncContextId = 0;
    VABufferID vaEncPipelineBufferId[4];
    static const VABufferID VA_H264ENC_BUFFER_INDEX_SEQ = 0;
    static const VABufferID VA_H264ENC_BUFFER_INDEX_PIC = 1;
    static const VABufferID VA_H264ENC_BUFFER_INDEX_SLICE = 2;
    static const VABufferID VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT = 3;
    static const UINT H264_MB_PIXEL_SIZE = 16;

    static void vaInfoCallback([[maybe_unused]] void *context, char *message);
    static void vaErrorCallback(void *context, char *message);

    //initialization of va
    bool initVaPipeline();
    bool initVaDisplay();
    bool ensureVaEncSupport();
    bool createVaSurfaces();
    bool initVaEncoder();
    bool initVaEncContext();

    bool encodeFrameVa(const uint8_t *data, uint32_t len);

    bool destroyVa();
    bool destroyVaEnc();
};

#endif //SNPSERVER_SNPENCODERVAH264_H
