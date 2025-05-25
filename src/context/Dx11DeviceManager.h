#ifndef SNPSERVER_DX11DEVICEMANAGER_H
#define SNPSERVER_DX11DEVICEMANAGER_H


#include <d3d11.h>

class Dx11DeviceManager {
public:
    bool initialize();
    ID3D11Device* getDevice();
    ID3D11DeviceContext* getContext();

private:
    ID3D11Device* device;
    ID3D11DeviceContext* context;
};


#endif //SNPSERVER_DX11DEVICEMANAGER_H
