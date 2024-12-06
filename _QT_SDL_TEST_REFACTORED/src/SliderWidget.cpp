#include "SliderWidget.h"


class SliderWidget : public QWidget {
    Q_OBJECT
    QSlider *slider;

public:
    explicit SliderWidget(int min, int max, int initialValue, QWidget *parent = nullptr)
        : QWidget(parent), slider(new QSlider(Qt::Horizontal, this)) {
        slider->setRange(min, max);
        slider->setValue(initialValue);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(slider);

        connect(slider, &QSlider::valueChanged, this, &SliderWidget::valueChanged);
    }

signals:
    void valueChanged(int value);
};
