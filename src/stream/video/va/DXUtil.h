#ifndef SNPSERVER_DXUTIL_H
#define SNPSERVER_DXUTIL_H


#include <dxgi.h>

class DXUtil {
public:
    static void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
};


#endif //SNPSERVER_DXUTIL_H
