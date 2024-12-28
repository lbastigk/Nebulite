#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QApplication>
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
    void pollMouseState();

    void convertSdlToImage(SDL_Renderer *renderer, int rendererWidth, int rendererHeight, int imageWidth, int imageHeight);
    void readTextureToImage(SDL_Texture *texture, int textureWidth, int textureHeight, int imageWidth, int imageHeight);

    QPoint getCursorPos(){return currentCursorPos;};
    Qt::MouseButtons getMouseState(){return mouseState;};
    int getWheelDelta(){int tmp = wheelDelta; wheelDelta=0; return tmp;};

private:
    Qt::MouseButtons mouseState;
    QPoint currentCursorPos;
    QColor currentPixelColor;

    QImage currentImage;
    int wheelDelta = 0;
protected:
    void wheelEvent(QWheelEvent *event) override;
};

#endif // IMAGEWIDGET_H
