#ifndef SNPSERVER_SNPSOURCEDDA_H
#define SNPSERVER_SNPSOURCEDDA_H

#include "stream/SnpComponent.h"
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <stdio.h>


struct SnpSourceDdaOptions : public SnpComponentOptions {
    uint32_t width;
    uint32_t height;
};

class SnpSourceDda : public SnpComponent {
public:
    explicit SnpSourceDda(const SnpSourceDdaOptions &options);
    ~SnpSourceDda() override;
    bool start() override;
    void stop() override;
private:
    bool initDda();
    bool destroyDda();
    bool captureFrame();
    ID3D11Device*           d3dDevice = nullptr;
    ID3D11DeviceContext*    d3dContext = nullptr;
    IDXGIOutputDuplication* duplication = nullptr;
    std::thread captureThread;
};


#endif //SNPSERVER_SNPSOURCEDDA_H
