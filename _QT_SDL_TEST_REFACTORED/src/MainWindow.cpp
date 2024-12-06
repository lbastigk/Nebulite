#include "MainWindow.h"
#include <QHBoxLayout>  // For organizing the main layout
#include <QVBoxLayout>  // For the control layout
#include "captureRendererContentToQImage.h"  // Helper function to capture SDL output as QImage


class MainWindow : public QWidget {
    Q_OBJECT
    ImageWidget *imageWidget;
    ButtonWidget *rotateButton;
    SliderWidget *speedSlider;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    QTimer *timer;
    double rotationSpeed;
    SDL_Texture *whiteTexture = nullptr;

public:
    MainWindow(QWidget *parent = nullptr)
        : QWidget(parent), renderer(nullptr), texture(nullptr), rotationSpeed(2.0) {
        if (!initializeSDL()) {
            qFatal("Failed to initialize SDL");
        }

        imageWidget = new ImageWidget(this);
        rotateButton = new ButtonWidget("Start Rotation", this);
        speedSlider = new SliderWidget(1, 10, 2, this);

        QVBoxLayout *controlLayout = new QVBoxLayout();
        controlLayout->addWidget(rotateButton);
        controlLayout->addWidget(speedSlider);

        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->addLayout(controlLayout);
        mainLayout->addWidget(imageWidget);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::updateImage);
        timer->start(16);

        connect(rotateButton, &ButtonWidget::buttonClicked, this, &MainWindow::toggleRotation);
        connect(speedSlider, &SliderWidget::valueChanged, this, &MainWindow::updateRotationSpeed);
    }

    ~MainWindow() {
        cleanupSDL();
    }

private:
    bool initializeSDL() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            qWarning("Unable to initialize SDL: %s", SDL_GetError());
            return false;
        }

        SDL_Window *sdlWindow = SDL_CreateWindow("SDL Hidden Window", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, SDL_WINDOW_HIDDEN);
        if (!sdlWindow) {
            qWarning("SDL Window could not be created! SDL_Error: %s", SDL_GetError());
            return false;
        }

        renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        if (!renderer) {
            qWarning("Renderer could not be created! SDL_Error: %s", SDL_GetError());
            SDL_DestroyWindow(sdlWindow);
            return false;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);
        if (!texture) {
            qWarning("Target texture could not be created! SDL_Error: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(sdlWindow);
            return false;
        }

        return true;
    }

    void cleanupSDL() {
        if (whiteTexture) SDL_DestroyTexture(whiteTexture);
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        SDL_Quit();
    }

    void renderContent() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw a green rectangle
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect greenRect = {200, 150, 240, 60};
        SDL_RenderFillRect(renderer, &greenRect);

        // Create and rotate the white square
        if (!whiteTexture) {
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 250, 250, 32, SDL_PIXELFORMAT_RGBA8888);
            SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 255, 255, 255, 255));
            whiteTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }

        static double angle = 0.0;
        angle += rotationSpeed;

        SDL_Rect whiteSquare = {295, 155, 250, 250};
        SDL_Point center = {125, 125};
        SDL_RenderCopyEx(renderer, whiteTexture, nullptr, &whiteSquare, angle, &center, SDL_FLIP_NONE);
    }

    void updateImage() {
        SDL_SetRenderTarget(renderer, texture);
        renderContent();

        QImage image = captureRendererContentToQImage(renderer, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);
        imageWidget->updateImage(image);
    }

    void toggleRotation() {
        rotationSpeed = (rotationSpeed == 0.0) ? 2.0 : 0.0;
        rotateButton->setText(rotationSpeed == 0.0 ? "Start Rotation" : "Stop Rotation");
    }

    void updateRotationSpeed(int value) {
        rotationSpeed = value * 0.2;
    }
};
