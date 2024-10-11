#include <iostream>
#include "stream/network/SnpSinkNetworkTcp.h"
#include "stream/SnpPipe.h"
#include "network/snp.pb.h"
#include "stream/SnpPipeFactory.h"

SnpSinkNetworkTcp* snpSinkNetworkTcp;
PipeMap pipeMap;

void handleSetupMessageCb(snp::Message* message) {
    //create a local config from message
    const snp::Setup& setup = message->setup();
    std::vector<snp::Component*> pipe;
    for(int i = 0; i < setup.component_size(); i++) {
        auto* component = new snp::Component();
        component->CopyFrom(setup.component(i));
        pipe.push_back(component);
    }
    pipeMap.insert(std::pair(setup.pipe_id(), pipe));

    std::vector<SnpPipe*> localPipes = SnpPipeFactory::createPipes(pipeMap);
    for (const auto &localPipe: localPipes) {
        localPipe->addComponentEnd(snpSinkNetworkTcp);
        localPipe->start();
    }
}

int main() {
    SnpSinkNetworkTcpOptions sinkOptions = {};
    sinkOptions.port = 9000;
    sinkOptions.host = "127.0.0.1";
    sinkOptions.handleSetupMessageCb = handleSetupMessageCb;
    snpSinkNetworkTcp = new SnpSinkNetworkTcp(sinkOptions);
    snpSinkNetworkTcp->start();

    while(TRUE) {
        Sleep(100);
    }

    return 0;
}

//s.run();

//    s.run();
//    while(1) {
//        for(auto& it : s.getClients()) {
//            std::string s = std::string("Hello Client");
//            it.second.send((uint8_t*)s.data(), s.size());
//        }
//        sleep(10);
//    }

//    VideoCaptureModesettingOptions options = {};
//    options.width = 100;
//    options.height = 100;
//    options.screenNumber = 10;
//
//    auto *videoCapture = new VideoCaptureModesetting(&options);
//    std::cout << "derived options:" << std::endl;
//    videoCapture->printOptions();
//
//    std::cout << "base options:" << std::endl;
//    videoCapture->VideoCapture::printOptions();