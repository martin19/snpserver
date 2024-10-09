#ifndef SNPSERVER_SNPCONFIG_H
#define SNPSERVER_SNPCONFIG_H

#include <string>
#include <QSettings>
#include "network/snappyv1.pb.h"

typedef std::map<uint32_t, std::vector<snappyv1::Component*>> PipeMap;

class SnpConfig {
public:
    SnpConfig();
    SnpConfig(const std::string& filename);
    void read(const std::string& filename);
    static void readPipesSection(QSettings *settings, PipeMap *pipes, const std::string &section);

    static uint32_t getPropertyUint(snappyv1::Component*, std::string name, uint32_t defaultValue);
    static bool getPropertyBool(snappyv1::Component*, std::string name, bool defaultValue);
    static double getPropertyDouble(snappyv1::Component*, std::string name, double defaultValue);
    static std::string getPropertyString(snappyv1::Component*, std::string name, std::string defaultValue);

private:
    PipeMap* localPipes;
    PipeMap* remotePipes;
public:
    PipeMap* getLocalPipes();
    PipeMap* getRemotePipes();

};

#endif //SNPSERVER_SNPCONFIG_H
