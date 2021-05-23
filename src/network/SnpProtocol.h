#ifndef SNPSERVER_SNPPROTOCOL_H
#define SNPSERVER_SNPPROTOCOL_H

#include "SnpClient.h"

class SnpProtocol {
public:
    static void sendServerInfo(SnpClient &c);
};

#endif //SNPSERVER_SNPPROTOCOL_H
