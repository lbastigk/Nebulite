#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QVBoxLayout>

class SliderWidget : public QWidget {
    Q_OBJECT

    QSlider *slider;

public:
    explicit SliderWidget(int min, int max, int initialValue, QWidget *parent = nullptr);

signals:
    void valueChanged(int value);
};

#endif // SLIDERWIDGET_H
