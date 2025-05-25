#ifndef SNPSERVER_SNPDATARAM_H
#define SNPSERVER_SNPDATARAM_H

#include "SnpData.h"
#include <cstdint>

class SnpDataRam : public SnpData {
public:
    SnpDataRam(uint8_t *data, uint32_t len, bool complete);
    uint8_t *getData();
    uint32_t getLen();
    bool getComplete();
private:
    uint8_t *data;
    uint32_t len;
    bool complete;
};


#endif //SNPSERVER_SNPDATARAM_H
