#include "SliderWidget.h"

SliderWidget::SliderWidget(int min, int max, int initialValue, bool isHorizontal, QWidget *parent)
    : QWidget(parent), slider(new QSlider(isHorizontal ? Qt::Horizontal : Qt::Vertical, this)) {
    slider->setRange(min, max);
    slider->setValue(initialValue);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(slider);

    connect(slider, &QSlider::valueChanged, this, &SliderWidget::valueChanged);
}
