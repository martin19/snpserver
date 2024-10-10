#ifndef SNPSERVER_SNPCOMPONENTREGISTRY_H
#define SNPSERVER_SNPCOMPONENTREGISTRY_H
#include <cstdint>
#include <set>
#include "network/snp.pb.h"

class SnpComponentRegistry {
private:
    std::unordered_map<std::string, int> componentMap;
    std::set<snp::Component*> localComponents;
    std::set<snp::Component*> remoteComponents;
public:
    SnpComponentRegistry();

    void registerLocalComponents();
    void registerLocalComponent(snp::Component* component);
    bool hasLocalComponent(snp::Component* component);
    void registerRemoteComponents(snp::Message* message);
    void registerRemoteComponent(snp::Component* component);
    bool hasRemoteComponent(snp::Component* component);
    std::set<snp::Component*> getLocalComponents();
    std::set<snp::Component*> getRemoteComponents();
    int getComponentId(const std::string& componentName) const;
};


#endif //SNPSERVER_SNPCOMPONENTREGISTRY_H
