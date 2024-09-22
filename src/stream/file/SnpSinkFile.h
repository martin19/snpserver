#ifndef SNPSERVER_SNPSINKFILE_H
#define SNPSERVER_SNPSINKFILE_H

#include <string>
#include <fstream>
#include "stream/SnpComponent.h"

struct SnpSinkFileOptions : public SnpComponentOptions {
    std::string fileName;
};

class SnpSinkFile : public SnpComponent {
public:
    explicit SnpSinkFile(const SnpSinkFileOptions &options);
    ~SnpSinkFile() override;

    bool start() override;
    void stop() override;

private:
    void onInputData(const uint8_t *data, int len, bool complete);
    std::vector<uint8_t> buffer;
    std::string fileName;
    std::ofstream output;
};

#endif //SNPSERVER_SNPSINKFILE_H
