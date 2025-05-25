#include "SnpSinkFile.h"
#include "stream/data/SnpDataRam.h"
#include <util/TimeUtil.h>
#include <fstream>

#define SNP_SINK_FILE_BUFFER_SIZE 500000

SnpSinkFile::SnpSinkFile(const SnpSinkFileOptions &options) : SnpComponent(options, "COMPONENT_OUTPUT_FILE") {
    fileName = options.fileName;

    addInputPort(new SnpPort());
    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkFile::onInputData, this, std::placeholders::_1, std::placeholders::_2));

    buffer.reserve(SNP_SINK_FILE_BUFFER_SIZE);
    buffer.clear();
}

SnpSinkFile::~SnpSinkFile() {
    //TODO:
}

void SnpSinkFile::onInputData(uint32_t pipeId, SnpData *data) {
    if(auto* ram = dynamic_cast<SnpDataRam*>(data)) {
        buffer.insert(buffer.end(), ram->getData(), ram->getData() + ram->getLen());
        if(ram->getComplete()) {
            output.write((char*)buffer.data(), buffer.size());
            buffer.clear();
        }
    }
}

bool SnpSinkFile::start() {
    SnpComponent::start();
    output = std::ofstream(fileName, std::ofstream::binary);
    return true;
}

void SnpSinkFile::stop() {
    SnpComponent::stop();
    output.close();
}

