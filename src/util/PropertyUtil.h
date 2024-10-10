#ifndef SNPSERVER_PROPERTYUTIL_H
#define SNPSERVER_PROPERTYUTIL_H


#include <network/snp.pb.h>

class PropertyUtil {
public:
    static void copySnpPropertyToProtocolProperty(snp::Property& dst, SnpProperty& src);
};


#endif //SNPSERVER_PROPERTYUTIL_H
