#include "ImageWidget.h"
#include <QPixmap>  // Needed for displaying QImage in QLabel

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent), label(new QLabel(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
}

void ImageWidget::updateImage(const QImage &image) {
    label->setPixmap(QPixmap::fromImage(image));
}
