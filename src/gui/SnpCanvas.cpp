//
// Created by marti on 15/09/2024.
//

#include "SnpCanvas.h"

SnpCanvas::SnpCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    qImage = new QImage(1920, 1080, QImage::Format_RGB888);
}

void SnpCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);

    // Set background color
    painter.fillRect(rect(), Qt::white);

//    // Draw a rectangle with red outline
//    painter.setPen(Qt::red);
//    painter.drawRect(50, 50, 200, 150);
//
//    // Fill a smaller rectangle with blue
//    painter.setBrush(Qt::blue);
//    painter.drawRect(100, 100, 100, 50);

    painter.drawImage(rect(), *qImage);
}

QImage *SnpCanvas::getQImage() const {
    return qImage;
}