#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ImageWidget.h"
#include "ButtonWidget.h"
#include "SliderWidget.h"
#include "ExplorerWidget.h"

#include <SDL2/SDL.h>

// Define window size macros for both SDL and Qt
#define SDL_WINDOW_WIDTH 640
#define SDL_WINDOW_HEIGHT 640
#define QT_WINDOW_WIDTH 1000 // Increased to accommodate the sidebar
#define QT_WINDOW_HEIGHT 640

class MainWindow : public QWidget {
    Q_OBJECT

    ImageWidget *imageWidget;
    ImageWidget *showcaseImageWidget;
    ButtonWidget *rotateButton;
    SliderWidget *speedSlider;
    ExplorerWidget *explorerWidget;

    SDL_Renderer *renderer;
    SDL_Texture *texture;
    QTimer *timer;

    double rotationSpeed;
    SDL_Texture *whiteTexture;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool initializeSDL();
    void cleanupSDL();
    void renderContent();
    void updateImage(ImageWidget &img);

private slots:
    void toggleRotation();
    void updateRotationSpeed(int value);
};

#endif // MAINWINDOW_H
