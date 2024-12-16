#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>

class ImageWidget : public QWidget {
    Q_OBJECT
    QLabel *label;

public:
    explicit ImageWidget(QWidget *parent = nullptr);
    void updateImage(const QImage &image);
};

#endif // IMAGEWIDGET_H
