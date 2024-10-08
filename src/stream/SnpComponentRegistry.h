#ifndef SNPSERVER_SNPCOMPONENTREGISTRY_H
#define SNPSERVER_SNPCOMPONENTREGISTRY_H
#include <cstdint>
#include <set>
#include "network/snappyv1.pb.h"

class SnpComponentRegistry {
private:
    std::set<snappyv1::Component*> localComponents;
    std::set<snappyv1::Component*> remoteComponents;
public:
    SnpComponentRegistry();

    void registerLocalComponents();
    void registerLocalComponent(snappyv1::Component* component);
    bool hasLocalComponent(snappyv1::Component* component);
    void registerRemoteComponents(snappyv1::Message* message);
    void registerRemoteComponent(snappyv1::Component* component);
    bool hasRemoteComponent(snappyv1::Component* component);
    std::set<snappyv1::Component*> getLocalComponents();
    std::set<snappyv1::Component*> getRemoteComponents();
};


#endif //SNPSERVER_SNPCOMPONENTREGISTRY_H
