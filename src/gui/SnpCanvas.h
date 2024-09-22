#ifndef SNPCLIENT_SNPCANVAS_H
#define SNPCLIENT_SNPCANVAS_H

#include <QWidget>
#include <QPainter>

class SnpCanvas : public QWidget {
public:
    SnpCanvas(QWidget *parent = nullptr);
    QImage *getQImage() const;
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void paintStatistics();

    QImage *qImage;
    uint32_t tsLastUpdate;
};


#endif //SNPCLIENT_SNPCANVAS_H
