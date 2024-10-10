#ifndef SNPSERVER_SNPCONFIG_H
#define SNPSERVER_SNPCONFIG_H

#include <string>
#include <QSettings>
#include "network/snp.pb.h"

typedef std::map<uint32_t, std::vector<snp::Component*>> PipeMap;

class SnpConfig {
public:
    SnpConfig();
    SnpConfig(const std::string& filename);
    void read(const std::string& filename);
    static void readPipesSection(QSettings *settings, PipeMap *pipes, const std::string &section);

    static uint32_t getPropertyUint(snp::Component*, std::string name, uint32_t defaultValue);
    static bool getPropertyBool(snp::Component*, std::string name, bool defaultValue);
    static double getPropertyDouble(snp::Component*, std::string name, double defaultValue);
    static std::string getPropertyString(snp::Component*, std::string name, std::string defaultValue);

private:
    PipeMap* localPipes;
    PipeMap* remotePipes;
public:
    PipeMap* getLocalPipes();
    PipeMap* getRemotePipes();

};

#endif //SNPSERVER_SNPCONFIG_H
