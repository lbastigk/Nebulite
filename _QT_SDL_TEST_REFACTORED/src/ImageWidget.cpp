#include "ImageWidget.h"
#include <QPixmap>  // Needed for displaying QImage in QLabel


class ImageWidget : public QWidget {
    QLabel *label;

public:
    explicit ImageWidget(QWidget *parent = nullptr)
        : QWidget(parent), label(new QLabel(this)) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(label);
    }

    void updateImage(const QImage &image) {
        label->setPixmap(QPixmap::fromImage(image));
    }
};

#include "ButtonWidget.moc
