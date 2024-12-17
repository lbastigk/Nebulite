#ifndef EXAMPLEWINDOW_H
#define EXAMPLEWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ImageWidget.h"
#include "ButtonWidget.h"
#include "SliderWidget.h"
#include "ExplorerWidget.h"

#include "Renderer.h"


// Define window size macros for both SDL and Qt
#define SDL_WINDOW_WIDTH 640
#define SDL_WINDOW_HEIGHT 640
#define QT_WINDOW_WIDTH 1000 // Increased to accommodate the sidebar
#define QT_WINDOW_HEIGHT 640

class _ExampleWindow : public QWidget {
    Q_OBJECT

    ImageWidget *imageWidget;
    ImageWidget *showcaseImageWidget;

    ButtonWidget *rotateButton;
    SliderWidget *speedSlider;
    ExplorerWidget *explorerWidget;

    QTimer *mainTimer;
    QTimer *othrTimer;

    Renderer nebuliteRenderer;
    Renderer nebuliteShowcaseRenderer;

    SDL_Texture *textureMain;
    SDL_Texture *textureOther;

public:
    explicit _ExampleWindow(QWidget *parent = nullptr);

private:
    void renderContent(Renderer &Renderer, SDL_Texture *texture);
    void updateImage(ImageWidget &img,Renderer &renderer,SDL_Texture *texture, float scalar);

private slots:

};

#endif // EXAMPLEWINDOW_H
