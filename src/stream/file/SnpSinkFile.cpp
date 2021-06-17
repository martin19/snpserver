#include "SnpSinkFile.h"
#include <util/TimeUtil.h>
#include <fstream>

#define SNP_SINK_FILE_BUFFER_SIZE 500000

SnpSinkFile::SnpSinkFile(const SnpSinkFileOptions &options) : SnpComponent(options) {
    componentName = "sinkFile";
    fileName = options.fileName;

    addInputPort(new SnpPort());
    getInputPort(0)->setOnDataCb(std::bind(&SnpSinkFile::onInputData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    buffer.reserve(SNP_SINK_FILE_BUFFER_SIZE);
    buffer.clear();
}

SnpSinkFile::~SnpSinkFile() {
    //TODO:
}

void SnpSinkFile::setEnabled(bool enabled) {
    if(enabled) {
        output = std::ofstream(fileName, std::ofstream::binary);
    } else {
        output.close();
    }
    SnpComponent::setEnabled(enabled);
}


void SnpSinkFile::onInputData(const uint8_t * inputBuffer, int inputLen, bool complete) {
    buffer.insert(buffer.end(), inputBuffer, inputBuffer + inputLen);
    if(complete) {
        output.write((char*)buffer.data(), buffer.size());
        buffer.clear();
    }
}

