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


SnpPipe *videoPipe = nullptr;
SnpCanvas *canvas = nullptr;
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

    SnpSourceNetworkTcpOptions sourceOptions = {};
    sourceOptions.streamId = 0;
    sourceOptions.port = 9000;
    sourceOptions.host = "127.0.0.1";
    sourceOptions.handleCapabilitiesMessageCb = handleCapabilitiesMessageCb;
    auto *source = new SnpSourceNetworkTcp(sourceOptions);
    source->start();

    //TODO: verify components are there and setup a local and remote pipe
//    videoPipe = SnpPipeFactory::createPipe(0, source, nullptr,
//                                           snappyv1::STREAM_MEDIUM_VIDEO,
//                                           snappyv1::STREAM_DIRECTION_INPUT,
//                                           snappyv1::STREAM_ENDPOINT_DISPLAY,
//                                           snappyv1::STREAM_ENCODING_H264_OPENH264);
//    videoPipe->start();

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