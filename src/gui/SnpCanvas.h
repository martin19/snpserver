#ifndef SNPCLIENT_SNPCANVAS_H
#define SNPCLIENT_SNPCANVAS_H

#include <QWidget>
#include <QPainter>

class SnpCanvas : public QWidget {
public:
    SnpCanvas(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QImage *qImage;
public:
    QImage *getQImage() const;

    void setQImage(QImage *qImage);
};


#endif //SNPCLIENT_SNPCANVAS_H
