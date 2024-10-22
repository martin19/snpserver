#include <QApplication>
#include <QMainWindow>
#include "gui/SnpCanvas.h"

#include <iostream>
#include "stream/SnpPipeFactory.h"
#include "stream/network/SnpSourceNetworkTcp.h"
#include "stream/SnpPipe.h"
#include "stream/output/SnpSinkDisplay.h"
#include "stream/SnpComponentRegistry.h"
#include "config/SnpConfig.h"


SnpPipe *videoPipe = nullptr;
SnpCanvas *canvas = nullptr;
SnpConfig* config;
SnpComponentRegistry* componentRegistry;
SnpSourceNetworkTcp* sourceNetworkTcp;

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

    sourceNetworkTcp->sendSetupMessage(config);
}

int runClient() {
    componentRegistry = new SnpComponentRegistry();
    config = new SnpConfig(R"(C:\Users\martin\CLionProjects\snpserver\snp.ini)");

    //setup local pipes
    std::vector<SnpPipe*> localPipes = SnpPipeFactory::createPipes(config->getLocalPipes());

    //add network source component
    SnpSourceNetworkTcpOptions sourceOptions = {};
    sourceOptions.port = 9000;
    sourceOptions.host = "127.0.0.1";
    sourceOptions.handleCapabilitiesMessageCb = handleCapabilitiesMessageCb;
    sourceNetworkTcp = new SnpSourceNetworkTcp(sourceOptions);
    for (const auto &pipe: localPipes) {
        if(!pipe->addComponentBegin(sourceNetworkTcp)) {
            return 1;
        }
        pipe->start();
    }

    //paint on every frame
    auto* sinkDisplay = dynamic_cast<SnpSinkDisplay *>(localPipes[0]->getComponents().back());
    sinkDisplay->getInputPort(0)->setOnDataCb([](auto pipeId, auto && data, auto && len, auto && PH3) {
        QImage* qImage = canvas->getQImage();
        memcpy(qImage->bits(), data, len);
        canvas->update();
    });

    return 0;
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