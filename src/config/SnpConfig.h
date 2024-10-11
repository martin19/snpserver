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
    static void readPipesSection(QSettings *settings, PipeMap& pipes, const std::string &section);

private:
    PipeMap localPipes;
    PipeMap remotePipes;
public:
    PipeMap& getLocalPipes();
    PipeMap& getRemotePipes();

};

#endif //SNPSERVER_SNPCONFIG_H
