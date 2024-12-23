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
#include <SDL2/SDL.h>

class ImageWidget : public QWidget {
    Q_OBJECT
    QLabel *label;

public:
    explicit ImageWidget(QWidget *parent = nullptr);
    void updateImage();
    void convertSdlToImage(SDL_Renderer *renderer, int rendererWidth, int rendererHeight, int imageWidth, int imageHeight);
    void mouseEvent(QMouseEvent *event);
private:
    QImage currentImage;
};

#endif // IMAGEWIDGET_H
