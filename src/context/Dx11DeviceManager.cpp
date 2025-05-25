#include "Dx11DeviceManager.h"

bool Dx11DeviceManager::initialize() {
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
    };

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
            nullptr,                    // default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &device,
            &featureLevel,
            &context
    );
    return SUCCEEDED(hr);
}

ID3D11Device *Dx11DeviceManager::getDevice() {
    return device;
}

ID3D11DeviceContext *Dx11DeviceManager::getContext() {
    return context;
}
