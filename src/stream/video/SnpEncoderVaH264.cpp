#include <util/assert.h>
#include <iostream>
#include <util/VideoUtil.h>
#include <cstring>
#include "util/loguru.h"
#include "SnpEncoderVaH264.h"
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "va/va_win32.h"
#include "d3d12.h"
#include <d3d12video.h>
#include <dxgi1_4.h>

//TODO: query d3d12 capabilities: (see https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloVAEncode/D3D12HelloVAEncode.cpp)
// https://www.vcodex.com/h264avc-picture-management/

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        LOG_F(ERROR,"%s failed with status %d,exit\n", __func__, va_status); \
        goto error;                                                        \
    }

#define CHECK_RESULT(result, func)                                  \
    if(result != true) {                                            \
        LOG_F(ERROR,"%s failed.\n", __func__);                      \
        goto error;                                                 \
    }

SnpEncoderVaH264::SnpEncoderVaH264(const SnpEncoderVaH264Options &options) : SnpComponent(options, "COMPONENT_ENCODER_INTEL") {
    addInputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_RGB));
    addOutputPort(new SnpPort(PORT_TYPE_BOTH, PORT_STREAM_TYPE_VIDEO_H264));
    addProperty(new SnpProperty("width", options.width));
    addProperty(new SnpProperty("height", options.height));
    //addProperty(new SnpProperty("fps", options.fps));
    addProperty(new SnpProperty("qp", options.qp));

    getInputPort(0)->setOnDataCb(std::bind(&SnpEncoderVaH264::onInputData, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

SnpEncoderVaH264::~SnpEncoderVaH264() {
    //TODO:
}

bool SnpEncoderVaH264::start() {
    SnpComponent::start();

    width = getProperty("width")->getValueUint32();
    height = getProperty("height")->getValueUint32();

//    frameWidthMbAligned = (width + 15) & (~15);
//    frameHeightMbAligned = (height + 15) & (~15);

    initVaEncoder();
    return true;
}

void SnpEncoderVaH264::stop() {
    SnpComponent::stop();
//    VaH264EncoderDestroy();
}

bool SnpEncoderVaH264::initVaEncoder() {
    initD3D12Pipeline();
    initVaPipeline();
}

bool SnpEncoderVaH264::initD3D12Pipeline() {
    bool result = true;
    HRESULT status;
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    status = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    CHECK_VASTATUS(status, "CreateDXGIFactory2");


    if (useWarpDevice) {
        status = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter))
        CHECK_VASTATUS(status, "factory->EnumWarpAdapter");
        status = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&device));
        CHECK_VASTATUS(status, "D3D12CreateDevice");
    } else {
        GetHardwareAdapter(factory.Get(), &adapter, true);
        status = D3D12CreateDevice(adapter.Get(),D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&device));
        CHECK_VASTATUS(status, "D3D12CreateDevice");
    }

    // Describe and create the command queue.
    //TODO below and above GetHardwareAdapter is part of DXSample.h

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaPipeline() {
    bool result;
    result = initVaDisplay();
    CHECK_RESULT(result, "initVaDisplay");
    result = ensureVaProcSupport();
    CHECK_RESULT(result, "ensureVaProcSupport");
    result = ensureVaEncSupport();
    CHECK_RESULT(result, "ensureVaEncSupport");
    result = createVaSurfaces();
    CHECK_RESULT(result, "createVaSurfaces");
    result = importRenderTargetsToVa();
    CHECK_RESULT(result, "importRenderTargetsToVa");
    result = initVaProcContext();
    CHECK_RESULT(result, "initVaProcContext");
    result = initVaEncContext();
    CHECK_RESULT(result, "initVaEncContext")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaDisplay() {
    bool result = true;
    VAStatus vaStatus;
    int majorVer, minorVer;

    vaDisplay = vaGetDisplayWin32(nullptr);
    vaSetInfoCallback(vaDisplay, reinterpret_cast<VAMessageCallback>(&SnpEncoderVaH264::vaInfoCallback), nullptr);
    vaSetErrorCallback(vaDisplay, reinterpret_cast<VAMessageCallback>(&SnpEncoderVaH264::vaErrorCallback), nullptr);

    vaStatus = vaInitialize(vaDisplay, &majorVer, &minorVer);
    CHECK_VASTATUS(vaStatus, "vaInitialize");

    LOG_F(INFO, "va display acquired (version %d.%d)", majorVer, minorVer);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::ensureVaProcSupport() {
    bool result = true;
    VAStatus vaStatus;

    bool supportsVideoProcessing = false;
    Microsoft::WRL::ComPtr<ID3D12VideoDevice> spVideoDevice;
    D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS vpMaxInputStreams = {};
    D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT dx12ProcCaps = {};

    int numEntrypoints = vaMaxNumEntrypoints(vaDisplay);
    std::vector<VAEntrypoint> entrypoints(numEntrypoints);
    vaStatus = vaQueryConfigEntrypoints(vaDisplay,VAProfileNone,entrypoints.data(),&numEntrypoints);
    CHECK_VASTATUS(vaStatus, "vaQueryConfigEntrypoints");

    for (int32_t i = 0; !supportsVideoProcessing && i < numEntrypoints; i++) {
        if (entrypoints[i] == VAEntrypointVideoProc)
            supportsVideoProcessing = true;
    }

    if (!supportsVideoProcessing) {
        LOG_F(ERROR, "VAEntrypointVideoProc not supported.");
        return false;
    }

    vaStatus = device->QueryInterface(IID_PPV_ARGS(spVideoDevice.GetAddressOf()));
    CHECK_VASTATUS(vaStatus, "device->QueryInterface");
    spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS, &vpMaxInputStreams, sizeof(vpMaxInputStreams));
    numVPRegions = std::min(vpMaxInputStreams.MaxInputStreams, (UINT)4);

    // Check VPBlit support for format DXGI_FORMAT_R8G8B8A8_UNORM
    dx12ProcCaps =
    {
        0, // NodeIndex
        { width, height, { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709}},
        D3D12_VIDEO_FIELD_TYPE_NONE,
        D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,
        { 30, 1 },
        { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 },
        D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,
        { 30, 1 },
    };

    vaStatus = spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, &dx12ProcCaps, sizeof(dx12ProcCaps));
    CHECK_VASTATUS(vaStatus, "spVideoDevice->CheckFeatureSupport");
    if ((dx12ProcCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) == 0) {
        LOG_F(ERROR, "VAEntrypointVideoProc not supported for format DXGI_FORMAT_R8G8B8A8_UNORM.");
        return false;
    }

    // Check VPBlit support for format DXGI_FORMAT_R8G8B8A8_UNORM -> DXGI_FORMAT_R8G8B8A8_NV12
    dx12ProcCaps.OutputFormat.Format = DXGI_FORMAT_NV12;
    dx12ProcCaps.OutputFormat.ColorSpace = DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709;
    vaStatus = spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, &dx12ProcCaps, sizeof(dx12ProcCaps));
    CHECK_VASTATUS(vaStatus, "spVideoDevice->CheckFeatureSupport");

    if ((dx12ProcCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) == 0) {
        LOG_F(ERROR, "VAEntrypointVideoProc not supported for conversion DXGI_FORMAT_R8G8B8A8_UNORM to DXGI_FORMAT_NV12.");
        return false;
    }

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::ensureVaEncSupport() {
    bool result = true;
    VAStatus vaStatus;

    bool supportsH264Enc = false;

    int numEntrypoints = vaMaxNumEntrypoints(vaDisplay);
    std::vector<VAEntrypoint> entrypoints(numEntrypoints);
    vaStatus = vaQueryConfigEntrypoints(vaDisplay,VAProfileH264Main,entrypoints.data(),&numEntrypoints);
    CHECK_VASTATUS(vaStatus, "vaQueryConfigEntrypoints for VAProfileH264Main");

    for (int32_t i = 0; !supportsH264Enc && i < numEntrypoints; i++) {
        if (entrypoints[i] == VAEntrypointEncSlice)
            supportsH264Enc = true;
    }

    if (!supportsH264Enc) {
        LOG_F(ERROR, "VAEntrypointEncSlice not supported for VAProfileH264Main.");
        return false;
    }

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::createVaSurfaces() {
    bool result = true;
    VAStatus vaStatus;

    VASurfaceAttrib createSurfacesAttribList[2] = {
        {
            VASurfaceAttribPixelFormat,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                    VAGenericValueTypeInteger,
                    // Based on the default render target
                    // format DXGI_FORMAT_R8G8B8A8_UNORM
                    VA_FOURCC_RGBA,
            },
        },
        {
            VASurfaceAttribMemoryType,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                    VAGenericValueTypeInteger,
                    VA_SURFACE_ATTRIB_MEM_TYPE_VA,
            },
        },
    };

    vaStatus = vaCreateSurfaces(vaDisplay,VA_RT_FORMAT_RGB32, width, height,
            vaRGBASurfaces,vaNumRGBASurfaces,createSurfacesAttribList,
            _countof(createSurfacesAttribList));
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces");

    createSurfacesAttribList[0].value.value.i = VA_FOURCC_NV12;
    vaStatus = vaCreateSurfaces(vaDisplay,VA_RT_FORMAT_YUV420, width, height,
            &vaSurfaceNV12,1,createSurfacesAttribList,_countof(createSurfacesAttribList));
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces");

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::importRenderTargetsToVa() {
    bool result = true;
    VAStatus vaStatus;

    VASurfaceAttrib createSurfacesAttribList[3] = {
        {
            VASurfaceAttribPixelFormat,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypeInteger,
                // Based on the default render target
                // format DXGI_FORMAT_R8G8B8A8_UNORM
                VA_FOURCC_RGBA,
            },
        },
        {
            VASurfaceAttribMemoryType,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypeInteger,
                // Using NTHandles for interop is the safe way of sharing resources between the
                // VADisplay D3D12 device and the app/sample D3D12 independent devices
                VA_SURFACE_ATTRIB_MEM_TYPE_NTHANDLE,
            },
        },
        {
        VASurfaceAttribExternalBufferDescriptor,
        VA_SURFACE_ATTRIB_SETTABLE,
        {
                VAGenericValueTypePointer,
                // createSurfacesAttribList[2].value.value.p is set in code below
                0,
            },
        },
    };

    HANDLE renderTargets[FrameCount];
    for (size_t i = 0; i < FrameCount; i++) {
        HRESULT hr = device->CreateSharedHandle(renderTargets[i].Get(), nullptr,GENERIC_ALL,
                                                  nullptr,&renderTargets[i]);
        CHECK_VASTATUS(hr, "device->CreateSharedHandle");
    }
    createSurfacesAttribList[2].value.value.p = renderTargets;

    // Creates VASurface objects by importing
    // handles of existing D3D12 resources
    vaStatus = vaCreateSurfaces(
            vaDisplay,
            VA_RT_FORMAT_RGB32,
            width,
            height,
            vaRenderTargets,
            FrameCount,
            createSurfacesAttribList,
            _countof(createSurfacesAttribList));
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces");

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaProcContext() {
    bool result = true;
    VAStatus vaStatus;

    vaStatus = vaCreateConfig(vaDisplay, VAProfileNone, VAEntrypointVideoProc, nullptr,
            0,&vaProcConfigId);
    CHECK_VASTATUS(vaStatus, "vaCreateConfig");

    // Context for color rgb to yuv conversion
    {
        vaStatus = vaCreateContext(vaDisplay,vaProcConfigId,(int)width,(int)height,
                VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaColorConvCtx);
        CHECK_VASTATUS(vaStatus, "vaCreateContext");

        vaStatus = vaCreateBuffer(vaDisplay,vaColorConvCtx,VAProcPipelineParameterBufferType,
                sizeof(VAProcPipelineParameterBuffer),1,nullptr,&vaColorConvBuf);
        CHECK_VASTATUS(vaStatus, "vaCreateBuffer");
    }

    // Context for single RGB -> RGB copy
    {
        vaStatus = vaCreateContext(vaDisplay,vaProcConfigId,(int)width,(int)height,
                VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaCopyCtx);
        CHECK_VASTATUS(vaStatus, "vaCreateContext");

        vaStatus = vaCreateBuffer(vaDisplay,vaCopyCtx,VAProcPipelineParameterBufferType,
                sizeof(VAProcPipelineParameterBuffer),1,nullptr,&vaCopyBuf);
        CHECK_VASTATUS(vaStatus, "vaCreateBuffer");
    }

    // Context for multiple RGB -> RGB blend
    {
        vaStatus = vaCreateContext(vaDisplay,vaProcConfigId,(int)width,(int)height,
                VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaBlendCtx);
        CHECK_VASTATUS(vaStatus, "vaCreateContext");

        vaStatus = vaCreateBuffer(vaDisplay,vaBlendCtx,VAProcPipelineParameterBufferType,
                sizeof(VAProcPipelineParameterBuffer),1,nullptr,&vaBlendBuf);
        CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

        vaStatus = vaQueryVideoProcPipelineCaps(vaDisplay,vaBlendCtx,nullptr,0,
                &procPipelineCaps);
        CHECK_VASTATUS(vaStatus, "vaQueryVideoProcPipelineCaps");

        int32_t XIncrement = (int32_t)width / numVPRegions;
        int32_t YIncrement = (int32_t)height / numVPRegions;
        int32_t XShift = 6;
        int32_t YShift = 6;
        for (INT i = 0; i < regionVariations; i++) {
            pBlendRegions[i][numVPRegions - 1].x = (int16_t)std::max(0, XIncrement - i);
            pBlendRegions[i][numVPRegions - 1].y = (int16_t)std::max(0, YIncrement - i);
            pBlendRegions[i][numVPRegions - 1].width = width / 2;
            pBlendRegions[i][numVPRegions - 1].height = height / 2;

            for (INT j = 0; j < numVPRegions - 1; j++) {
                pBlendRegions[i][j].x = std::min((int16_t)width, (int16_t)(j * XIncrement + i * 0.25f * j * XShift));
                pBlendRegions[i][j].y = std::min((int16_t)height, (int16_t)(j * YIncrement + i * 0.5f * YShift));
                pBlendRegions[i][j].width = regionsSizeRatio * XIncrement;
                pBlendRegions[i][j].height = regionsSizeRatio * YIncrement;
            }

            colors[i][0] = 0.0f;
            colors[i][1] = 0.2f;
            colors[i][2] = 0.1f + std::min(0.4f, i*0.0125f);
            colors[i][3] = 1.0f;
        }
    }

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaEncContext() {
    bool result = true;
    VAStatus vaStatus;

    finalEncodedBitstream.open("output_bitstream.h264", std::ios::binary);
    vaStatus = vaCreateConfig(vaDisplay,VAProfileH264Main,VAEntrypointEncSlice,
            nullptr,0,&vaEncConfigId);
    CHECK_VASTATUS(vaStatus, "vaCreateConfig");

    vaStatus = vaCreateContext(vaDisplay,vaEncConfigId,(int)width,(int)height,
            VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaEncContextId);
    CHECK_VASTATUS(vaStatus, "vaCreateContext");

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncSequenceParameterBufferType,
            sizeof(VAEncSequenceParameterBufferH264),1, nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncPictureParameterBufferType,
            sizeof(VAEncPictureParameterBufferH264),1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId, VAEncSliceParameterBufferType,
            sizeof(VAEncSliceParameterBufferH264),1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    // Worst case within reason assume same as uncompressed surface
    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncCodedBufferType,
            width * height * 3,1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer");

    return result;
error:
    return result;
}

void SnpEncoderVaH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    if(!isRunning()) return;
//    this->VaH264EncoderEncode(data, len);
}

void SnpEncoderVaH264::vaInfoCallback(void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}

void SnpEncoderVaH264::vaErrorCallback(void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}