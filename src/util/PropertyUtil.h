#ifndef SNPSERVER_PROPERTYUTIL_H
#define SNPSERVER_PROPERTYUTIL_H


#include <network/snappyv1.pb.h>

class PropertyUtil {
public:
    static void copySnpPropertyToProtocolProperty(snappyv1::Property& dst, SnpProperty& src);
};


#endif //SNPSERVER_PROPERTYUTIL_H
