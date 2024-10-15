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
   initVaPipeline();
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

    //TODO: query d3d12 capabilities: (see https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloVAEncode/D3D12HelloVAEncode.cpp)
    // Check VPBlit support for format DXGI_FORMAT_R8G8B8A8_UNORM
    // Check VPBlit support for format DXGI_FORMAT_R8G8B8A8_UNORM -> DXGI_FORMAT_R8G8B8A8_NV12

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

    //TODO: uses directx12 api.. either include the api or try working without it.
    // The value here is an array of num_surfaces pointers to HANDLE so
    // each handle can be associated with the corresponding output surface
    // in the call to vaCreateSurfaces
//    HANDLE renderTargets[FrameCount];
//    for (size_t i = 0; i < FrameCount; i++)
//    {
//        HRESULT hr = m_device->CreateSharedHandle(m_renderTargets[i].Get(),
//                                                  nullptr,
//                                                  GENERIC_ALL,
//                                                  nullptr,
//                                                  &renderTargets[i]);
//        ThrowIfFailed(hr);
//    }
//    createSurfacesAttribList[2].value.value.p = renderTargets;

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