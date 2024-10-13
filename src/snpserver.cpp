#include <iostream>
#include "stream/network/SnpSinkNetworkTcp.h"
#include "stream/SnpPipe.h"
#include "network/snp.pb.h"
#include "stream/SnpPipeFactory.h"
#include "stream/video/SnpEncoderAmfH264.h"
#include "stream/video/SnpSourceDummy.h"
#include "stream/file/SnpSinkFile.h"

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

int main2() {


    SnpPipeOptions pipeOptions = {};
    pipeOptions.name = "amd";
    SnpPipe pipe(pipeOptions, 0);

    SnpSourceDummyOptions sourceDummyOptions = {};
    sourceDummyOptions.width = 1920;
    sourceDummyOptions.height = 1080;
    sourceDummyOptions.fps = 60.0;
    SnpSourceDummy snpSourceDummy(sourceDummyOptions);

    SnpEncoderAmfH264Options amfH264Options = {};
    amfH264Options.width = 1920;
    amfH264Options.height = 1080;
    amfH264Options.fps = 60.0;
    SnpEncoderAmfH264 snpEncoderAmfH264(amfH264Options);

    SnpSinkFileOptions sinkFileOptions = {};
    sinkFileOptions.fileName = "test1.h264";
    SnpSinkFile snpSinkFile(sinkFileOptions);

    //TODO: every component needs a pipeId (verify this) maybe a constructor parameter is necessary
    snpSourceDummy.setPipeId(0);
    snpEncoderAmfH264.setPipeId(0);
    snpSinkFile.setPipeId(0);
    pipe.addComponentEnd(&snpSourceDummy);
    pipe.addComponentEnd(&snpEncoderAmfH264);
    pipe.addComponentEnd(&snpSinkFile);

    pipe.start();

    while(TRUE) {
        Sleep(100);
    }
    return 0;

    //play raw h264 file:
    //vlc file:///p:/snp/snpserver/cmake-build-release-windows/test1.h264 --demux h264
}