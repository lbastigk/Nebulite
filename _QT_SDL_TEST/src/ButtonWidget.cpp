#include "ButtonWidget.h"

ButtonWidget::ButtonWidget(const QString &buttonText, QWidget *parent)
    : QWidget(parent), button(new QPushButton(buttonText, this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, &ButtonWidget::emitButtonClicked);
}

void ButtonWidget::setText(const QString &text) {
    button->setText(text);  // Forward the call to the QPushButton instance
}

void ButtonWidget::emitButtonClicked() {
    emit buttonClicked();  // Emit the custom buttonClicked signal
}
