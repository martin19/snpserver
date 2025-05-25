#ifndef SNPSERVER_SNPCONTEXT_H
#define SNPSERVER_SNPCONTEXT_H


#include "Dx11DeviceManager.h"

class SnpContext {
public:
    SnpContext();
    virtual ~SnpContext();
private:
    Dx11DeviceManager *dx11DeviceManager;
public:
    [[nodiscard]] Dx11DeviceManager *getDx11DeviceManager() const;
};


#endif //SNPSERVER_SNPCONTEXT_H
