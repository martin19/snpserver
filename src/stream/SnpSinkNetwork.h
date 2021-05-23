#ifndef SNPSERVER_SNPSINKNETWORK_H
#define SNPSERVER_SNPSINKNETWORK_H

#include <network/SnpClient.h>
#include "SnpSink.h"

struct SnpSinkNetworkOptions : public SnpSinkOptions {
    SnpClient *client;
};

class SnpSinkNetwork : public SnpSink {
private:
    SnpClient *client;
    std::vector<uint8_t> buffer;
public:
    explicit SnpSinkNetwork(const SnpSinkNetworkOptions &options);
    ~SnpSinkNetwork() override;

    void process(uint8_t *data, int len, bool complete);
};

#endif //SNPSERVER_SNPSINKNETWORK_H
