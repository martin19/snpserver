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

SnpPipe *videoPipe = nullptr;
SnpCanvas *canvas = nullptr;

int runClient() {
    SnpSourceNetworkTcpOptions sourceOptions = {};
    sourceOptions.streamId = 0;
    sourceOptions.port = 9000;
    sourceOptions.host = "127.0.0.1";
    auto *source = new SnpSourceNetworkTcp(sourceOptions);

    videoPipe = SnpPipeFactory::createPipe(0, source, nullptr,
                                           snappyv1::STREAM_MEDIUM_VIDEO,
                                           snappyv1::STREAM_DIRECTION_INPUT,
                                           snappyv1::STREAM_ENDPOINT_DISPLAY,
                                           snappyv1::STREAM_ENCODING_H264_OPENH264);
    videoPipe->setEnabled(true);
    videoPipe->start();

    //paint on every frame
    auto* sinkDisplay = dynamic_cast<SnpSinkDisplay *>(videoPipe->getComponents().back());
    sinkDisplay->getInputPort(0)->setOnDataCb([](auto && data, auto && len, auto && PH3) {
        QImage* qImage = canvas->getQImage();
        memcpy(qImage->bits(), data, len);
        canvas->update();
    });
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