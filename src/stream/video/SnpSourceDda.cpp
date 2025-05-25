#include <thread>
#include "SnpSourceDda.h"
#include "stream/data/SnpDataDx11.h"
#include "stream/SnpPipe.h"

SnpSourceDda::SnpSourceDda(const SnpSourceDdaOptions &options) : SnpComponent(options, "COMPONENT_CAPTURE_VIDEO_DDA") {
    addOutputPort(new SnpPort(PORT_STREAM_TYPE_VIDEO_RGBA));
    //    addProperty(new SnpProperty("fps", options.fps));
    //    addProperty(new SnpProperty("width", options.width));
    //    addProperty(new SnpProperty("height", options.height));
    //    addProperty(new SnpProperty("boxCount", options.boxCount));
    //    addProperty(new SnpProperty("boxSpeed", options.boxSpeed));
}

SnpSourceDda::~SnpSourceDda() {

}

bool SnpSourceDda::start() {
    SnpComponent::start();
    LOG_F(INFO, "starting DDA video source");
    initDda();
    captureThread = std::thread(&SnpSourceDda::captureFrame, this);
    return true;
}

void SnpSourceDda::stop() {
    SnpComponent::stop();
    destroyDda();
}

bool SnpSourceDda::initDda() {
    HRESULT hr;

    // Create D3D11 device
//    D3D_FEATURE_LEVEL featureLevel;
//    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
//                           &d3dDevice, &featureLevel, &d3dContext);
//
//    if (FAILED(hr)) {
//        LOG_F(ERROR, "Failed to create D3D11 device: 0x%lx\n", hr);
//        return false;
//    }

    SnpPipe *pipe = getOwner();
    d3dDevice = pipe->getContext()->getDx11DeviceManager()->getDevice();
    d3dContext = pipe->getContext()->getDx11DeviceManager()->getContext();


    // Get DXGI device
    IDXGIDevice* dxgiDevice = nullptr;
    hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) return false;

    IDXGIAdapter* adapter = nullptr;
    dxgiDevice->GetAdapter(&adapter);
    dxgiDevice->Release();

    IDXGIOutput* output = nullptr;
    adapter->EnumOutputs(0, &output);  // Use primary monitor
    adapter->Release();

    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release();
    if (FAILED(hr)) return false;

    hr = output1->DuplicateOutput(d3dDevice, &duplication);
    output1->Release();

    if (FAILED(hr)) {
        LOG_F(ERROR, "DuplicateOutput failed: 0x%lx\n", hr);
        return false;
    }

    return true;
}

bool SnpSourceDda::destroyDda() {
    if(duplication != nullptr) duplication->Release();
    if(d3dContext != nullptr) d3dContext->Release();
    if(d3dDevice != nullptr) d3dDevice->Release();
    return true;
}

bool SnpSourceDda::captureFrame() {
    SnpPort * outputPort = this->getOutputPort(0);
    ID3D11Texture2D* outTexture;
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;

    while(this->isRunning()) {
        HRESULT hr = duplication->AcquireNextFrame(1000, &frameInfo, &desktopResource);
        if (FAILED(hr)) {
            if (hr == DXGI_ERROR_WAIT_TIMEOUT)
                return false;
            LOG_F(WARNING, "AcquireNextFrame failed: 0x%lx\n", hr);
            return false;
        }

        hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void **) &outTexture);

        SnpDataDx11 dataDx11(outTexture);
        outputPort->onData(getPipeId(), &dataDx11);

        outTexture->Release();
        desktopResource->Release();
        duplication->ReleaseFrame();
    }

    return true;
}