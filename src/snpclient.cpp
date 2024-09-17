#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include "gui/SnpCanvas.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("SnpClient");
    SnpCanvas *canvas = new SnpCanvas;
    window.setCentralWidget(canvas);
    window.resize(800, 600);
    window.show();
    return app.exec();
}
