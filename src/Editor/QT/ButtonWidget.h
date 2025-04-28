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

    void setText(const QString &text);  // Declare setText method

signals:
    void buttonClicked();

private slots:
    void emitButtonClicked();  // Slot to emit buttonClicked signal
};

#endif // BUTTONWIDGET_H
