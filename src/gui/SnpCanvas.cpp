#include "SnpCanvas.h"
#include "util/TimeUtil.h"

SnpCanvas::SnpCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    qImage = new QImage(1920, 1080, QImage::Format_RGBA8888);
}

void SnpCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    painter.drawImage(rect(), *qImage);
    paintStatistics();
}

void SnpCanvas::paintStatistics() {
    QPainter painter(this);
    uint32_t tsNow = TimeUtil::getTimeNowMs();
    uint32_t frameMs = tsNow - tsLastUpdate;
    double fps = 1000.0 / frameMs;
    painter.setPen(Qt::green);
    QString fpsStr;
    QTextStream fpsStream(&fpsStr);
    fpsStream.setRealNumberNotation(QTextStream::FixedNotation);
    fpsStream.setRealNumberPrecision(2);
    fpsStream << "fps: " << fps;
    painter.fillRect(rect().right() - 100, rect().top(), 100, 100, Qt::black);
    painter.drawText(rect().right() - 100, rect().top() + 30, fpsStr);
    tsLastUpdate = tsNow;
}

QImage *SnpCanvas::getQImage() const {
    return qImage;
}