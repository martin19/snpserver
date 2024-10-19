#include <iostream>
#include <util/VideoUtil.h>
#include <cstring>
#include "util/loguru.h"
#include "SnpEncoderVaH264.h"
#include "va/va.h"
#include "va/va_enc_h264.h"
#include "va/va_win32.h"
#include <d3d12.h>
#include <directx/d3d12video.h>
#include <directx/d3dx12.h>
#include <cassert>
#include "va/DXUtil.h"

//TODO: query d3d12 capabilities: (see https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloVAEncode/D3D12HelloVAEncode.cpp)
// https://www.vcodex.com/h264avc-picture-management/

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        result = false;                                                 \
        LOG_F(ERROR,"%s failed with status %d,exit\n", __func__, (int)va_status); \
        goto error;                                                        \
    }

#define CHECK_RESULT(result, func)                                  \
    if(result != true) {                                            \
        LOG_F(ERROR,"%s failed.\n", __func__);                      \
        goto error;                                                 \
    }

void SnpEncoderVaH264::vaInfoCallback([[maybe_unused]] void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
}

void SnpEncoderVaH264::vaErrorCallback([[maybe_unused]] void* context, char* message) {
    LOG_F(INFO, "VAAPI: %s", message);
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

    initVaEncoder();
    return true;
}

void SnpEncoderVaH264::stop() {
    SnpComponent::stop();
    destroyVa();
}

bool SnpEncoderVaH264::initVaEncoder() {
    initD3D12Pipeline();
    initVaPipeline();
}

bool SnpEncoderVaH264::initD3D12Pipeline() {
    bool result = true;
//    HRESULT status;
//    UINT dxgiFactoryFlags = 0;
//    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
//    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
//    ComPtr<IDXGISwapChain1> swapChain;
//
//#if defined(_DEBUG)
//    // Enable the debug layer (requires the Graphics Tools "optional feature").
//    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
//    {
//        ComPtr<ID3D12Debug> debugController;
//        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
//        {
//            debugController->EnableDebugLayer();
//
//            // Enable additional debug layers.
//            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
//        }
//    }
//#endif
//
//    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
//    status = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
//    CHECK_VASTATUS(status, "CreateDXGIFactory2");
//
//
//    if (useWarpDevice) {
//        status = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter))
//        CHECK_VASTATUS(status, "factory->EnumWarpAdapter");
//        status = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&device));
//        CHECK_VASTATUS(status, "D3D12CreateDevice");
//    } else {
//        DXUtil::GetHardwareAdapter(factory.Get(), &adapter, true);
//        status = D3D12CreateDevice(adapter.Get(),D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&device));
//        CHECK_VASTATUS(status, "D3D12CreateDevice");
//    }
//
//    // Describe and create the command queue.
//    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//
//    status = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
//    CHECK_VASTATUS(status, "device->CreateCommandQueue");
//
//    // Describe and create the swap chain.
//    swapChainDesc.BufferCount = FrameCount;
//    swapChainDesc.Width = width;
//    swapChainDesc.Height = height;
//    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//    swapChainDesc.SampleDesc.Count = 1;
//
//
//    status = factory->CreateSwapChainForHwnd(
//            commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
//            Win32Application::GetHwnd(),
//            &swapChainDesc,
//            nullptr,
//            nullptr,
//            &swapChain
//    ));
//
//    //TODO: unclear if we need DX12 pipeline. Probably encoder works without it.

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::performVaEncodeFrame(VASurfaceID dstSurface, VABufferID dstCompressedbit) {
    VAStatus status;
    bool result = true;

    status = vaBeginPicture(vaDisplay, vaEncContextId, dstSurface);
    CHECK_VASTATUS(status, "vaBeginPicture")

    // VAEncSequenceParameterBufferH264
    {
        VAEncSequenceParameterBufferH264* pMappedBuf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ], (void**)&pMappedBuf);
        CHECK_VASTATUS(status, "vaMapBuffer")
        memset(pMappedBuf, 0, sizeof(*pMappedBuf));

        // Level 4.1 as per H.264 codec standard
        pMappedBuf->level_idc = 41;

        // 2 * fps_num for 30fps
        pMappedBuf->time_scale = 2 * 30;
        // fps_den
        pMappedBuf->num_units_in_tick = 1;

        pMappedBuf->intra_idr_period = 1;
        pMappedBuf->seq_fields.bits.pic_order_cnt_type = 2;

        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ]);
        CHECK_VASTATUS(status, "vaUnMapBuffer")
    }

    // VAEncPictureParameterBufferH264
    {
        VAEncPictureParameterBufferH264* pMappedBuf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC], (void**)&pMappedBuf);
        CHECK_VASTATUS(status, "vaMapBuffer")
        memset(pMappedBuf, 0, sizeof(*pMappedBuf));

        pMappedBuf->pic_fields.bits.idr_pic_flag = 1;
        // We can use always 0 as each frame is an IDR which resets the GOP
        pMappedBuf->CurrPic.TopFieldOrderCnt = 0;
        pMappedBuf->CurrPic.picture_id = dstSurface;
        pMappedBuf->coded_buf = dstCompressedbit;

        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC]);
        CHECK_VASTATUS(status, "vaUnMapBuffer")
    }

    // VAEncSliceParameterBufferH264
    {
        VAEncSliceParameterBufferH264* pMappedBuf;
        status = vaMapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE], (void**)&pMappedBuf);
        CHECK_VASTATUS(status, "vaMapBuffer")
        memset(pMappedBuf, 0, sizeof(*pMappedBuf));

        pMappedBuf->num_macroblocks = (width / H264_MB_PIXEL_SIZE * height / H264_MB_PIXEL_SIZE);
        pMappedBuf->slice_type = 2; // intra slice
        status = vaUnmapBuffer(vaDisplay, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE]);
        CHECK_VASTATUS(status, "vaUnMapBuffer")
    }

    // Apply encode, send the first 3 seq, pic, slice buffers
    vaRenderPicture(vaDisplay, vaEncContextId, vaEncPipelineBufferId, 3);

    status = vaEndPicture(vaDisplay, vaEncContextId);
    CHECK_VASTATUS(status, "vaEndPicture")

    // Wait for completion on GPU for the indicated VABuffer/VASurface
    // Attempt vaSyncBuffer if VA driver implements it first
    status = vaSyncBuffer(vaDisplay, dstCompressedbit, VA_TIMEOUT_INFINITE);
    if (status != VA_STATUS_ERROR_UNIMPLEMENTED) {
        CHECK_VASTATUS(status, "vaSyncBuffer")
    } else {
        // Legacy API call otherwise
        status = vaSyncSurface(vaDisplay, dstSurface);
        CHECK_VASTATUS(status, "vaSyncSurface")
    }

    // Flush encoded bitstream to disk
    {
        VACodedBufferSegment *buf_list, *buf;
        status = vaMapBuffer(vaDisplay, dstCompressedbit, (void**)&buf_list);
        CHECK_VASTATUS(status, "vaMapBuffer");
        for (buf = buf_list; buf; buf = (VACodedBufferSegment*) buf->next) {
            //TODO: write to output port instead
            finalEncodedBitstream.write(reinterpret_cast<char*>(buf->buf), buf->size);
        }

        status = vaUnmapBuffer(vaDisplay, dstCompressedbit);
        CHECK_VASTATUS(status, "vaUnMapBuffer");
    }

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::performVaWorkload() {
    VAStatus status;
    bool result = true;

    // Copy the cleared render target with solid color into m_vaRGBASurfaces[i]
    for (UINT i = 0; i < numVPRegions; i++) {
        performVaBlit(vaCopyCtx, vaCopyBuf, &vaRenderTargets[frameIndex], 1, nullptr, nullptr, vaRGBASurfaces[i], 1.0f);
    }

    // Blit the source surface m_NumVPRegions times in different regions in the output surface

    // Blend, translate and scale src_regions into dst_regions of the render target
    performVaBlit(vaBlendCtx, vaBlendBuf, vaRGBASurfaces, numVPRegions, pBlendRegions[curRegionVariation],
                  pBlendRegions[curRegionVariation], vaRenderTargets[frameIndex], alphaBlend);
    curRegionVariation = ((curRegionVariation + 1) % regionVariations);

    // Color convert RGB into NV12 for encode
    performVaBlit(vaColorConvCtx, vaColorConvBuf, &vaRenderTargets[frameIndex], 1, nullptr, nullptr, vaSurfaceNV12, 1.0f);

    // Encode render target frame into an H.264 bitstream
    performVaEncodeFrame(vaSurfaceNV12, vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT]);

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::performVaBlit(VAContextID context, VABufferID buffer, VASurfaceID* pInSurfaces,
                                     UINT inSurfacesCount, VARectangle* pSrcRegions, VARectangle* pDstRegions,
                                     VASurfaceID dstSurface, float alpha) {
    VAStatus status;
    bool result = true;

    assert(inSurfacesCount <= vaNumRGBASurfaces);

    status = vaBeginPicture(vaDisplay, context, dstSurface);
    CHECK_VASTATUS(status, "vaBeginPicture")

    for (size_t i = 0; i < inSurfacesCount; i++) {
        VAProcPipelineParameterBuffer* pipeline_param;
        status = vaMapBuffer(vaDisplay, buffer, (void**)&pipeline_param);
        memset(pipeline_param, 0, sizeof(VAProcPipelineParameterBuffer));
        CHECK_VASTATUS(status, "vaMapBuffer")
        pipeline_param->surface = pInSurfaces[i];
        if (pSrcRegions)
            pipeline_param->surface_region = &pSrcRegions[i];
        if (pDstRegions)
            pipeline_param->output_region = &pDstRegions[i];

        // Check the VA platform can perform global alpha
        // blend using the queried capabilities previously
        VABlendState blend;
        if (procPipelineCaps.blend_flags & VA_BLEND_GLOBAL_ALPHA) {
            memset(&blend, 0, sizeof(VABlendState));
            blend.flags = VA_BLEND_GLOBAL_ALPHA;
            blend.global_alpha = alpha;
            pipeline_param->blend_state = &blend;
        }

        status = vaUnmapBuffer(vaDisplay, buffer);
        CHECK_VASTATUS(status, "vaUnMapBuffer")

        // Apply VPBlit
        vaRenderPicture(vaDisplay, context, &buffer, 1);
    }
    status = vaEndPicture(vaDisplay, context);
    CHECK_VASTATUS(status, "vaEndPicture")

    // Wait for completion on GPU for the indicated VASurface
    status = vaSyncSurface(vaDisplay, dstSurface);
    CHECK_VASTATUS(status, "vaSyncSurface")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaPipeline() {
    bool result;
    result = initVaDisplay();
    CHECK_RESULT(result, "initVaDisplay")
    result = ensureVaProcSupport();
    CHECK_RESULT(result, "ensureVaProcSupport")
    result = ensureVaEncSupport();
    CHECK_RESULT(result, "ensureVaEncSupport")
    result = createVaSurfaces();
    CHECK_RESULT(result, "createVaSurfaces")
    result = importRenderTargetsToVa();
    CHECK_RESULT(result, "importRenderTargetsToVa")
    result = initVaProcContext();
    CHECK_RESULT(result, "initVaProcContext")
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
    CHECK_VASTATUS(vaStatus, "vaInitialize")

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
    CHECK_VASTATUS(vaStatus, "vaQueryConfigEntrypoints")

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
    CHECK_VASTATUS(vaStatus, "spVideoDevice->CheckFeatureSupport")
    if ((dx12ProcCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) == 0) {
        LOG_F(ERROR, "VAEntrypointVideoProc not supported for format DXGI_FORMAT_R8G8B8A8_UNORM.");
        return false;
    }

    // Check VPBlit support for format DXGI_FORMAT_R8G8B8A8_UNORM -> DXGI_FORMAT_R8G8B8A8_NV12
    dx12ProcCaps.OutputFormat.Format = DXGI_FORMAT_NV12;
    dx12ProcCaps.OutputFormat.ColorSpace = DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709;
    vaStatus = spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, &dx12ProcCaps, sizeof(dx12ProcCaps));
    CHECK_VASTATUS(vaStatus, "spVideoDevice->CheckFeatureSupport")

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
    CHECK_VASTATUS(vaStatus, "vaQueryConfigEntrypoints for VAProfileH264Main")

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
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces")

    createSurfacesAttribList[0].value.value.i = VA_FOURCC_NV12;
    vaStatus = vaCreateSurfaces(vaDisplay,VA_RT_FORMAT_YUV420, width, height,
            &vaSurfaceNV12,1,createSurfacesAttribList,_countof(createSurfacesAttribList));
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces")

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

    HANDLE renderTargets2[FrameCount];
    for (size_t i = 0; i < FrameCount; i++) {
        HRESULT hr = device->CreateSharedHandle(renderTargets[i].Get(), nullptr,GENERIC_ALL,
                                                  nullptr,&renderTargets2[i]);
        CHECK_VASTATUS(hr, "device->CreateSharedHandle")
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
    CHECK_VASTATUS(vaStatus, "vaCreateSurfaces")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::initVaProcContext() {
    bool result = true;
    VAStatus vaStatus;

    vaStatus = vaCreateConfig(vaDisplay, VAProfileNone, VAEntrypointVideoProc, nullptr,
            0,&vaProcConfigId);
    CHECK_VASTATUS(vaStatus, "vaCreateConfig")

    // Context for color rgb to yuv conversion
    {
        vaStatus = vaCreateContext(vaDisplay,vaProcConfigId,(int)width,(int)height,
                VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaColorConvCtx);
        CHECK_VASTATUS(vaStatus, "vaCreateContext")

        vaStatus = vaCreateBuffer(vaDisplay,vaColorConvCtx,VAProcPipelineParameterBufferType,
                sizeof(VAProcPipelineParameterBuffer),1,nullptr,&vaColorConvBuf);
        CHECK_VASTATUS(vaStatus, "vaCreateBuffer")
    }

    // Context for single RGB -> RGB copy
    {
        vaStatus = vaCreateContext(vaDisplay,vaProcConfigId,(int)width,(int)height,
                VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaCopyCtx);
        CHECK_VASTATUS(vaStatus, "vaCreateContext")

        vaStatus = vaCreateBuffer(vaDisplay,vaCopyCtx,VAProcPipelineParameterBufferType,
                sizeof(VAProcPipelineParameterBuffer),1,nullptr,&vaCopyBuf);
        CHECK_VASTATUS(vaStatus, "vaCreateBuffer")
    }

    // Context for multiple RGB -> RGB blend
    {
        vaStatus = vaCreateContext(vaDisplay,vaProcConfigId,(int)width,(int)height,
                VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaBlendCtx);
        CHECK_VASTATUS(vaStatus, "vaCreateContext")

        vaStatus = vaCreateBuffer(vaDisplay,vaBlendCtx,VAProcPipelineParameterBufferType,
                sizeof(VAProcPipelineParameterBuffer),1,nullptr,&vaBlendBuf);
        CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

        vaStatus = vaQueryVideoProcPipelineCaps(vaDisplay,vaBlendCtx,nullptr,0,
                &procPipelineCaps);
        CHECK_VASTATUS(vaStatus, "vaQueryVideoProcPipelineCaps")

        auto XIncrement = (int32_t)(width / numVPRegions);
        auto YIncrement = (int32_t)(height / numVPRegions);
        int32_t XShift = 6;
        int32_t YShift = 6;
        for (INT i = 0; i < regionVariations; i++) {
            pBlendRegions[i][numVPRegions - 1].x = (int16_t)std::max(0, XIncrement - i);
            pBlendRegions[i][numVPRegions - 1].y = (int16_t)std::max(0, YIncrement - i);
            pBlendRegions[i][numVPRegions - 1].width = width / 2;
            pBlendRegions[i][numVPRegions - 1].height = height / 2;

            for (INT j = 0; j < numVPRegions - 1; j++) {
                pBlendRegions[i][j].x = std::min((int16_t)width, (int16_t)((float)j * (float)XIncrement + (float)i * 0.25f * (float)j * (float)XShift));
                pBlendRegions[i][j].y = std::min((int16_t)height, (int16_t)((float)j * (float)YIncrement + (float)i * 0.5f * (float)YShift));
                pBlendRegions[i][j].width = (uint16_t)(regionsSizeRatio * (float)XIncrement);
                pBlendRegions[i][j].height = (uint16_t)(regionsSizeRatio * (float)YIncrement);
            }

            colors[i][0] = 0.0f;
            colors[i][1] = 0.2f;
            colors[i][2] = 0.1f + std::min(0.4f, (float)i*0.0125f);
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
    CHECK_VASTATUS(vaStatus, "vaCreateConfig")

    vaStatus = vaCreateContext(vaDisplay,vaEncConfigId,(int)width,(int)height,
            VA_PROGRESSIVE,vaRenderTargets,FrameCount,&vaEncContextId);
    CHECK_VASTATUS(vaStatus, "vaCreateContext")

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncSequenceParameterBufferType,
            sizeof(VAEncSequenceParameterBufferH264),1, nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SEQ]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncPictureParameterBufferType,
            sizeof(VAEncPictureParameterBufferH264),1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_PIC]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId, VAEncSliceParameterBufferType,
            sizeof(VAEncSliceParameterBufferH264),1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_SLICE]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    // Worst case within reason assume same as uncompressed surface
    vaStatus = vaCreateBuffer(vaDisplay,vaEncContextId,VAEncCodedBufferType,
            width * height * 3,1,nullptr,
            &vaEncPipelineBufferId[VA_H264ENC_BUFFER_INDEX_COMPRESSED_BIT]);
    CHECK_VASTATUS(vaStatus, "vaCreateBuffer")

    return result;
error:
    return result;
}

void SnpEncoderVaH264::onInputData(uint32_t pipeId, const uint8_t *data, uint32_t len, bool complete) {
    if(!isRunning()) return;
    encodeFrameVa(data, len);
}

bool SnpEncoderVaH264::encodeFrameVa(const uint8_t *data, uint32_t len) {
    bool result = true;
    VAStatus status;
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };

    // Record all the commands we need to render the scene into the command list.
    // In this case, clear the render target with a predefined color
    result = populateCommandList();
    CHECK_RESULT(result, "populateCommandList");

    // Execute the command list.
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Before calling PerformVAWorkload, we must ensure the following:
    //  1. The D3D12 resources to be used must be in D3D12_RESOURCE_STATE_COMMON state
    //      * PopulateCommandList is already transitioning the resource to D3D12_RESOURCE_STATE_PRESENT
    //          which happens to also match the definition of D3D12_RESOURCE_STATE_COMMON
    //  2. The D3D12 resources must not have any pending GPU operations
    //      * Call WaitForPreviousFrame below for this end, to wait for the ExecuteCommandLists below
    //          that clears this render target with a predefined solid color.

    result = waitForPreviousFrame();
    CHECK_RESULT(result, "waitForPreviousFrame");

    // Perform the VA workload on the current render target
    // The VA driver internally manages any other state transitions and it is expected that
    // PerformVAWorkload calls vaSyncSurface, which ensures the affected resources are
    // back in COMMON state and all the GPU work flushed and finished on them
    // Currently only m_VARenderTargets[m_frameIndex] is used in the VA workload,
    // transition it back to present mode for the call below.

    result = performVaWorkload();
    CHECK_RESULT(result, "performVaWorkload");

    result = waitForPreviousFrame();
    CHECK_RESULT(result, "waitForPreviousFrame");

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::populateCommandList() {
    bool result = true;
    HRESULT status;
    CD3DX12_RESOURCE_BARRIER barrier1;
    CD3DX12_RESOURCE_BARRIER barrier2;
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), (int)frameIndex, m_rtvDescriptorSize);

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    status = commandAllocator->Reset();
    CHECK_VASTATUS(status, "commandAllocator->Reset")

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    status = commandList->Reset(commandAllocator.Get(), pipelineState.Get());
    CHECK_VASTATUS(status, "commandList->Reset")

    // Indicate that the back buffer will be used as a render target.
    barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    // Record commands.
    commandList->ClearRenderTargetView(rtvHandle, colors[curRegionVariation], 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);

    status = commandList->Close();
    CHECK_VASTATUS(status, "commandList->Close")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::waitForPreviousFrame() {
    bool result = true;
    HRESULT status;

    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    status = commandQueue->Signal(fence.Get(), fenceValue);


    fenceValue++;

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < fenceValue) {
        status = fence->SetEventOnCompletion(fenceValue, fenceEvent);
        CHECK_VASTATUS(status, fence->SetEventOnCompletion);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
    return result;
error:
    return result;
}

bool SnpEncoderVaH264::destroyVa() {
    bool result = true;
    VAStatus status;

    destroyVaProc();
    destroyVaEnc();

    // Destroy VA Common

    status = vaDestroySurfaces(vaDisplay, vaRenderTargets, FrameCount);
    CHECK_VASTATUS(status, "vaDestroySurfaces")

    status = vaDestroySurfaces(vaDisplay, vaRGBASurfaces, vaNumRGBASurfaces);
    CHECK_VASTATUS(status, "vaDestroySurfaces")

    status = vaDestroySurfaces(vaDisplay, &vaSurfaceNV12, 1);
    CHECK_VASTATUS(status, "vaDestroySurfaces")

    vaTerminate(vaDisplay);
    CHECK_VASTATUS(status, "vaTerminate")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::destroyVaProc() {
    bool result = true;
    VAStatus status;

    status = vaDestroyConfig(vaDisplay, vaProcConfigId);
    CHECK_VASTATUS(status, "vaDestroyConfig")

    status = vaDestroyContext(vaDisplay, vaCopyCtx);
    CHECK_VASTATUS(status, "vaDestroyContext")

    status = vaDestroyContext(vaDisplay, vaBlendCtx);
    CHECK_VASTATUS(status, "vaDestroyContext")

    status = vaDestroyContext(vaDisplay, vaColorConvCtx);
    CHECK_VASTATUS(status, "vaDestroyContext")

    status = vaDestroyBuffer(vaDisplay, vaCopyBuf);
    CHECK_VASTATUS(status, "vaDestroyBuffer")

    status = vaDestroyBuffer(vaDisplay, vaColorConvBuf);
    CHECK_VASTATUS(status, "vaDestroyBuffer")

    status = vaDestroyBuffer(vaDisplay, vaBlendBuf);
    CHECK_VASTATUS(status, "vaDestroyBuffer")

    return result;
error:
    return result;
}

bool SnpEncoderVaH264::destroyVaEnc() {
    bool result = true;
    VAStatus status;

    status = vaDestroyConfig(vaDisplay, vaEncConfigId);
    CHECK_VASTATUS(status, "vaDestroyConfig")

    status = vaDestroyContext(vaDisplay, vaEncContextId);
    CHECK_VASTATUS(status, "vaDestroyContext")

    for (UINT i = 0; i < _countof(vaEncPipelineBufferId); i++) {
        vaDestroyBuffer(vaDisplay, vaEncPipelineBufferId[i]);
        CHECK_VASTATUS(status, "vaDestroyBuffer")
    }

    finalEncodedBitstream.flush();
    finalEncodedBitstream.close();
    return result;
error:
    return result;
}