#include <iostream>
#include "stream/network/SnpSinkNetworkTcp.h"
#include "stream/SnpPipe.h"
#include "network/snp.pb.h"
#include "config/SnpConfig.h"
#include "stream/SnpPipeFactory.h"

SnpConfig* snpConfig;
SnpSinkNetworkTcp* snpSinkNetworkTcp;

void handleSetupMessageCb(snp::Message* message) {
    auto* pipeMap = new PipeMap();

    //create a local config from message
    const snp::Setup& setup = message->setup();
    std::vector<snp::Component*> pipe;
    pipeMap->insert(std::pair(setup.pipe_id(), pipe));
    for(int i = 0; i < setup.component_size(); i++) {
        auto* component = new snp::Component();
        component->CopyFrom(setup.component(i));
        pipe.push_back(component);
    }

    std::vector<SnpPipe*>* localPipes = SnpPipeFactory::createPipes(pipeMap);

    //TODO: hen-egg problem. source needs to be created and used before connecting pipes to it.
    //creating ports for each connected pipe works on client side but in server case we need to
    //connect pipes to a running tcp component - rethink this and fix it.
    for (const auto &pipe: *localPipes) {
        SnpComponent* first = *pipe->getComponents().begin();
        sourceOptions.portStreamTypes.push_back(first->getInputPort(0)->getStreamType());
    }
    for (const auto &pipe: *localPipes) {
        pipe->addComponentEnd(snpSinkNetworkTcp);
        pipe->start();
    }
}

int main() {
    SnpSinkNetworkTcpOptions sinkOptions = {};
    sinkOptions.port = 9000;
    sinkOptions.host = "127.0.0.1";
    sinkOptions.handleSetupMessageCb = handleSetupMessageCb;
    sinkOptions.portStreamTypes = std::vector<PortStreamType>();
    snpSinkNetworkTcp = new SnpSinkNetworkTcp(sinkOptions);
    snpSinkNetworkTcp->start();

//    SnpPipe *videoPipe = SnpPipeFactory::createPipe(0, nullptr, sink,
//                                                    snp::STREAM_MEDIUM_VIDEO,
//                                                    snp::STREAM_DIRECTION_OUTPUT,
//                                                    snp::STREAM_ENDPOINT_VIDEO_DUMMY,
//                                                    snp::STREAM_ENCODING_H264_OPENH264);
//    videoPipe->start();
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