#ifndef SNPCLIENT_SNPCANVAS_H
#define SNPCLIENT_SNPCANVAS_H

#include <QWidget>
#include <QPainter>

class SnpCanvas : public QWidget {
public:
    SnpCanvas(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
};


#endif //SNPCLIENT_SNPCANVAS_H
