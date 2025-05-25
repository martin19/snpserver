#include "SnpDataDx11.h"

SnpDataDx11::SnpDataDx11(ID3D11Texture2D *texture) : texture(texture) {}

ID3D11Texture2D *SnpDataDx11::getTexture() {
    return texture;
}
