#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include "gui/SnpCanvas.h"

#include <iostream>
#include <network/SnpWebsocket.h>
#include "util/loguru.h"
#include "stream/SnpPipeFactory.h"
#include "stream/network/SnpSourceNetworkTcp.h"
#include "stream/SnpPipe.h"
#include "stream/output/SnpSinkDisplay.h"
#include "stream/video/SnpSourceDummy.h"
#include "stream/SnpComponentRegistry.h"
#include "config/SnpConfig.h"


SnpPipe *videoPipe = nullptr;
SnpCanvas *canvas = nullptr;
SnpConfig* config;
SnpComponentRegistry* componentRegistry;

void handleCapabilitiesMessageCb(snp::Message* message) {
    componentRegistry->registerRemoteComponents(message);

    auto localComponents = componentRegistry->getLocalComponents();
    auto remoteComponents = componentRegistry->getRemoteComponents();

    std::cout << "Local components:" << std::endl;
    for (const auto &component: localComponents) {
        std::cout << component->componenttype() << std::endl;
    }
    std::cout << "Remote components:" << std::endl;
    for (const auto &component: remoteComponents) {
        std::cout << component->componenttype() << std::endl;
    }
}

int runClient() {
    componentRegistry = new SnpComponentRegistry();
    config = new SnpConfig(R"(P:\snp\snpserver\snp.ini)");

    //setup local pipes
    std::vector<SnpPipe*>* localPipes = SnpPipeFactory::createPipes(config, "local");

    //add network source component
    SnpSourceNetworkTcpOptions sourceOptions = {};
    sourceOptions.port = 9000;
    sourceOptions.host = "127.0.0.1";
    sourceOptions.handleCapabilitiesMessageCb = handleCapabilitiesMessageCb;
    sourceOptions.portStreamTypes = std::vector<PortStreamType>();
    for (const auto &pipe: *localPipes) {
        SnpComponent* first = *pipe->getComponents().begin();
        sourceOptions.portStreamTypes.push_back(first->getInputPort(0)->getStreamType());
    }
    auto *source = new SnpSourceNetworkTcp(sourceOptions);
    for (const auto &pipe: *localPipes) {
        pipe->addComponentBegin(source);
        pipe->start();
    }

    //paint on every frame
    auto* sinkDisplay = dynamic_cast<SnpSinkDisplay *>(videoPipe->getComponents().back());
    sinkDisplay->getInputPort(0)->setOnDataCb([](auto pipeId, auto && data, auto && len, auto && PH3) {
        QImage* qImage = canvas->getQImage();
        memcpy(qImage->bits(), data, len);
        canvas->update();
    });

    source->sendSetupMessage(config);
    //TODO: send remote config to server and setup pipe using createPipes
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("SnpClient");
    canvas = new SnpCanvas;
    window.setCentralWidget(canvas);
    window.resize(1920/2, 1080/2);
    window.show();
    runClient();
    return app.exec();
}