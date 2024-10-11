#include <QSettings>
#include "SnpConfig.h"
#include "network/snp.pb.h"

SnpConfig::SnpConfig() {}

SnpConfig::SnpConfig(const std::string& filename) {
    read(filename);
}


void SnpConfig::read(const std::string& filename) {
    QSettings settings(QString(filename.c_str()), QSettings::IniFormat);
    readPipesSection(&settings, localPipes, "local");
    readPipesSection(&settings, remotePipes, "remote");
}

void SnpConfig::readPipesSection(QSettings* settings, PipeMap& pipes, const std::string& section) {

    QString s = QString::fromStdString(section);

    for(int i = 0; i < 256; i++) {
        QString pipeKey = QString("%1/pipe/%2").arg(s, QString::number(i));
        if(!settings->contains(pipeKey)) {
            break;
        }

        std::vector<snp::Component*> pipe;

        //read all components for this pipe
        for(int j = 0; j < 256; j++) {
            QString componentKey = QString("%1/pipe/%2/component/%3").arg(s, QString::number(i), QString::number(j));
            if(!settings->contains(componentKey)) {
                break;
            }

            int32_t componentType = settings->value(componentKey, -1).toInt();
            auto c = new snp::Component();
            pipe.push_back(c);

            c->set_componenttype(static_cast<snp::ComponentType>(componentType));
            for(int k = 0; k < 256; k++) {
                QString propertyNameKey = QString("%1/property/%2/name").arg(componentKey, QString::number(k));
                QString propertyValueKey = QString("%1/property/%2/value").arg(componentKey, QString::number(k));
                QString propertyTypeKey = QString("%1/property/%2/type").arg(componentKey, QString::number(k));
                if(!settings->contains(propertyNameKey) ||
                   !settings->contains(propertyValueKey) ||
                   !settings->contains(propertyTypeKey)) {
                    break;
                }
                auto p = c->add_property();
                QString propertyName = settings->value(propertyNameKey).toString();
                uint32_t propertyType = settings->value(propertyTypeKey).toUInt();
                p->set_name(propertyName.toStdString());
                p->set_type(static_cast<snp::PropertyType>(propertyType));
                if(propertyType == 0) {
                    p->mutable_value_string()->set_value(settings->value(propertyValueKey).toString().toStdString());
                } else if(propertyType == 1) {
                    p->mutable_value_bool()->set_value(settings->value(propertyValueKey).toBool());
                } else if(propertyType == 2) {
                    p->mutable_value_uint32()->set_value(settings->value(propertyValueKey).toUInt());
                } else if(propertyType == 3) {
                    p->mutable_value_double()->set_value(settings->value(propertyValueKey).toDouble());
                }
            }
        }

        pipes.insert(std::make_pair(i, pipe));
    }
}

PipeMap& SnpConfig::getLocalPipes() {
    return localPipes;
}

PipeMap& SnpConfig::getRemotePipes() {
    return remotePipes;
}

