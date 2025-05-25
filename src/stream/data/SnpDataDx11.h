//
// Created by marti on 25/05/2025.
//

#ifndef SNPSERVER_SNPDATADX11_H
#define SNPSERVER_SNPDATADX11_H


#include <d3d11.h>
#include "SnpData.h"

class SnpDataDx11 : public SnpData {
public:
    SnpDataDx11(ID3D11Texture2D *texture);
public:
    ID3D11Texture2D* getTexture();
private:
    ID3D11Texture2D* texture;
};


#endif //SNPSERVER_SNPDATADX11_H
