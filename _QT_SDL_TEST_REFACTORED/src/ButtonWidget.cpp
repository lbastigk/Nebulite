#include "ButtonWidget.h"


class ButtonWidget : public QWidget {
    Q_OBJECT
    QPushButton *button;

public:
    explicit ButtonWidget(const QString &buttonText, QWidget *parent = nullptr)
        : QWidget(parent), button(new QPushButton(buttonText, this)) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(button);

        connect(button, &QPushButton::clicked, this, &ButtonWidget::buttonClicked);
    }

signals:
    void buttonClicked();
};
