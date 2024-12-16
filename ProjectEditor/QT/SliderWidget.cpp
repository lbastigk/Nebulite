#include "SliderWidget.h"

SliderWidget::SliderWidget(int min, int max, int initialValue, QWidget *parent)
    : QWidget(parent), slider(new QSlider(Qt::Horizontal, this)) {
    slider->setRange(min, max);
    slider->setValue(initialValue);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(slider);

    connect(slider, &QSlider::valueChanged, this, &SliderWidget::valueChanged);
}
