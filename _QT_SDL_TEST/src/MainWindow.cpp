#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "captureRendererContentToQImage.h"  // Helper function to capture SDL output as QImage

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), renderer(nullptr), texture(nullptr), rotationSpeed(2.0), whiteTexture(nullptr) {
    if (!initializeSDL()) {
        qFatal("Failed to initialize SDL");
    }

    // Initialize widgets
    imageWidget             = new ImageWidget(this);   // Main Image Widget
    showcaseImageWidget     = new ImageWidget(this);
    rotateButton            = new ButtonWidget("Start Rotation", this);
    speedSlider             = new SliderWidget(1, 10, 2, this);
    explorerWidget          = new ExplorerWidget(this);

    // Setup sdl placeholder widget
    showcaseImageWidget->setFixedSize(200, 150);   // Set size for SDL placeholder
    showcaseImageWidget->updateImage(QImage());    // Set a placeholder image (empty or static)

    // Create control layout (vertical)
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->addWidget(showcaseImageWidget);  // SDL placeholder on top
    controlLayout->addWidget(rotateButton);
    controlLayout->addWidget(speedSlider);

    // Create main layout (horizontal)
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(explorerWidget);    // Explorer on the left
    mainLayout->addLayout(controlLayout);     // Controls in the middle (with SDL placeholder)
    mainLayout->addWidget(imageWidget);       // Main image on the right

    setLayout(mainLayout);

    // Initialize timer and connect signals
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateImage);
    timer->start(16);

    connect(rotateButton, &ButtonWidget::buttonClicked, this, &MainWindow::toggleRotation);
    connect(speedSlider, &SliderWidget::valueChanged, this, &MainWindow::updateRotationSpeed);
    connect(explorerWidget, &ExplorerWidget::fileSelected, [](const QString &filePath) {
        qDebug() << "Selected file:" << filePath;
    });
}




MainWindow::~MainWindow() {
    cleanupSDL();
}

bool MainWindow::initializeSDL() {
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

void MainWindow::cleanupSDL() {
    if (whiteTexture) SDL_DestroyTexture(whiteTexture);
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void MainWindow::renderContent() {
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

void MainWindow::updateImage() {
    SDL_SetRenderTarget(renderer, texture);
    renderContent();

    QImage image = captureRendererContentToQImage(renderer, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);
    imageWidget->updateImage(image);
}

void MainWindow::toggleRotation() {
    rotationSpeed = (rotationSpeed == 0.0) ? 2.0 : 0.0;
    rotateButton->setText(rotationSpeed == 0.0 ? "Start Rotation" : "Stop Rotation");
}

void MainWindow::updateRotationSpeed(int value) {
    rotationSpeed = value * 0.2;
}
