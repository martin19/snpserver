#include "SnpSinkFile.h"
#include <util/TimeUtil.h>
#include <fstream>

#define SNP_SINK_FILE_BUFFER_SIZE 500000

SnpSinkFile::SnpSinkFile(const SnpSinkFileOptions &options) : SnpComponent(options, "COMPONENT_OUTPUT_FILE") {
    fileName = options.fileName;

    addInputPort(new SnpPort());
    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkFile::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    buffer.reserve(SNP_SINK_FILE_BUFFER_SIZE);
    buffer.clear();
}

SnpSinkFile::~SnpSinkFile() {
    //TODO:
}

void SnpSinkFile::onInputData(uint32_t pipeId, const uint8_t * inputBuffer, int inputLen, bool complete) {
    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
        output.write((char*)buffer.data(), buffer.size());
        buffer.clear();
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

