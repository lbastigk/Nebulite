#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class ButtonWidget : public QWidget {
    Q_OBJECT
    QPushButton *button;

public:
    explicit ButtonWidget(const QString &buttonText, QWidget *parent = nullptr);

signals:
    void buttonClicked();
};

#endif // BUTTONWIDGET_H
