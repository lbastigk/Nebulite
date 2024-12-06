#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ImageWidget.h"
#include "ButtonWidget.h"
#include "SliderWidget.h"
#include <SDL2/SDL.h>

class MainWindow : public QWidget {
    Q_OBJECT

    ImageWidget *imageWidget;
    ButtonWidget *rotateButton;
    SliderWidget *speedSlider;

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
    void updateImage();

private slots:
    void toggleRotation();
    void updateRotationSpeed(int value);
};

#endif // MAINWINDOW_H
