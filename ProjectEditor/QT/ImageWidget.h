#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>  // Needed for displaying QImage in QLabel
#include <QFile>
#include <QBuffer>
#include <QByteArray>
#include <QMouseEvent>
#include <QCursor>
#include <SDL2/SDL.h>

class ImageWidget : public QWidget {
    Q_OBJECT
    QLabel *label;

public:
    explicit ImageWidget(QWidget *parent = nullptr);
    void updateImage();
    void convertSdlToImage(SDL_Renderer *renderer, int rendererWidth, int rendererHeight, int imageWidth, int imageHeight);
    void pollMousePosition();

    QPoint getCursorPos(){return currentCursorPos;};
private:
    QPoint currentCursorPos;
    QColor currentPixelColor;

    QImage currentImage;
};

#endif // IMAGEWIDGET_H
