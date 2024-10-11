#ifndef SNPSERVER_PROPERTYUTIL_H
#define SNPSERVER_PROPERTYUTIL_H


#include <network/snp.pb.h>

class PropertyUtil {
public:
    static void copySnpPropertyToProtocolProperty(snp::Property& dst, SnpProperty& src);
    static uint32_t getPropertyUint(snp::Component*, std::string name, uint32_t defaultValue);
    static bool getPropertyBool(snp::Component*, std::string name, bool defaultValue);
    static double getPropertyDouble(snp::Component*, std::string name, double defaultValue);
    static std::string getPropertyString(snp::Component*, std::string name, std::string defaultValue);
};


#endif //SNPSERVER_PROPERTYUTIL_H
