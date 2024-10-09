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

void handleCapabilitiesMessageCb(snappyv1::Message* message) {
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

    SnpSourceNetworkTcpOptions sourceOptions = {};
    sourceOptions.streamId = 0;
    sourceOptions.port = 9000;
    sourceOptions.host = "127.0.0.1";
    sourceOptions.handleCapabilitiesMessageCb = handleCapabilitiesMessageCb;
    auto *source = new SnpSourceNetworkTcp(sourceOptions);
    source->start();



    //setup local pipes
    //TODO: connect components in pipe and connect to source pipe (network, local file etc.)
    //TODO: send remote config to server and setup pipe using createPipes
    std::vector<SnpPipe*>* pipes = SnpPipeFactory::createPipes(config, "local");
    for (const auto &pipe: *pipes) {
        pipe->start();
    }

    //paint on every frame
//    auto* sinkDisplay = dynamic_cast<SnpSinkDisplay *>(videoPipe->getComponents().back());
//    sinkDisplay->getInputPort(0)->setOnDataCb([](auto && data, auto && len, auto && PH3) {
//        QImage* qImage = canvas->getQImage();
//        memcpy(qImage->bits(), data, len);
//        canvas->update();
//    });
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