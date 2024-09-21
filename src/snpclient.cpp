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

SnpPipe *videoPipe = nullptr;

int runClient() {
    SnpSourceNetworkTcpOptions sourceOptions = {};
    sourceOptions.streamId = 0;
    sourceOptions.port = 9000;
    sourceOptions.host = "127.0.0.1";
    auto *sink = new SnpSourceNetworkTcp(sourceOptions);
    videoPipe = SnpPipeFactory::createPipe(0, nullptr, sink,
                                                    snappyv1::STREAM_MEDIUM_VIDEO,
                                                    snappyv1::STREAM_DIRECTION_INPUT,
                                                    snappyv1::STREAM_ENDPOINT_VIDEO_DUMMY,
                                                    snappyv1::STREAM_ENCODING_H264_OPENH264);
    videoPipe->setEnabled(true);
    videoPipe->start();
    while(TRUE) {
        Sleep(100);
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("SnpClient");
    auto *canvas = new SnpCanvas;
    window.setCentralWidget(canvas);
    window.resize(800, 600);
    window.show();
    runClient();

    //paint on every frame
    auto* sinkDisplay = dynamic_cast<SnpSinkDisplay *>(videoPipe->getComponents().back());
    canvas->setQImage(sinkDisplay->getQImage());
    sinkDisplay->setOnFrameCb([canvas]() {
       canvas->update();
    });

    return app.exec();
}