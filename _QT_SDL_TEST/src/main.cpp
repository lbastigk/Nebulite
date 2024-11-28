#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QImage>
#include <QTimer>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <SDL2/SDL.h>

// Define window size macros for both SDL and Qt
#define SDL_WINDOW_WIDTH 640
#define SDL_WINDOW_HEIGHT 640
#define QT_WINDOW_WIDTH 1000 // Increased to accommodate the sidebar
#define QT_WINDOW_HEIGHT 640

QImage captureRendererContentToQImage(SDL_Renderer *renderer, int width, int height) {
    // Create an SDL surface with a pixel format that matches what SDL uses
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
        qWarning("Failed to create surface for QImage conversion. SDL_Error: %s", SDL_GetError());
        return QImage();
    }

    // Get the pixel format of the renderer's current target
    SDL_RendererInfo rendererInfo;
    SDL_GetRendererInfo(renderer, &rendererInfo);
    Uint32 renderFormat = rendererInfo.texture_formats[0];  // Default format used by the renderer

    // Read pixels from the texture, not from the renderer
    if (SDL_RenderReadPixels(renderer, NULL, renderFormat, surface->pixels, surface->pitch) != 0) {
        qWarning("Failed to read pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return QImage();
    }

    // Convert the SDL surface into a QImage
    QImage image((uchar*)surface->pixels, width, height, surface->pitch, QImage::Format_RGBA8888);
    QImage result = image.copy(); // Make a copy to manage the data safely outside SDL's lifecycle

    // Free the SDL surface
    SDL_FreeSurface(surface);
    return result;
}

class ImageWidget : public QWidget {
    QLabel *label;
    SDL_Window *sdlWindow;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    QTimer *timer; // Timer to update images
    QPushButton *rotateButton; // Button to start/stop rotation
    QSlider *speedSlider; // Slider for rotation speed
    double rotationSpeed;

public:
    ImageWidget(QWidget *parent = nullptr)
        : QWidget(parent), sdlWindow(nullptr), renderer(nullptr), texture(nullptr), rotationSpeed(2.0) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            qWarning("Unable to initialize SDL: %s", SDL_GetError());
            return;
        }

        // Create SDL window for display using the SDL_WINDOW_WIDTH and SDL_WINDOW_HEIGHT macros
        sdlWindow = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!sdlWindow) {
            qWarning("SDL Window could not be created! SDL_Error: %s", SDL_GetError());
            SDL_Quit();
            return;
        }

        // Create renderer for SDL window using the SDL_WINDOW_WIDTH and SDL_WINDOW_HEIGHT macros
        renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        if (!renderer) {
            qWarning("Renderer could not be created! SDL_Error: %s", SDL_GetError());
            SDL_DestroyWindow(sdlWindow);
            SDL_Quit();
            return;
        }

        // Create target texture for off-screen rendering using the SDL_WINDOW_WIDTH and SDL_WINDOW_HEIGHT macros
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);
        if (!texture) {
            qWarning("Target texture could not be created! SDL_Error: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(sdlWindow);
            SDL_Quit();
            return;
        }

        // Hide the SDL window
        SDL_HideWindow(sdlWindow);

        // Set up the Qt window layout
        QHBoxLayout *layout = new QHBoxLayout(this);
        QVBoxLayout *leftPanel = new QVBoxLayout(); // Left sidebar layout

        // Add buttons and sliders to the left sidebar
        rotateButton = new QPushButton("Start Rotation", this);
        leftPanel->addWidget(rotateButton);

        speedSlider = new QSlider(Qt::Horizontal, this);
        speedSlider->setRange(1, 10); // Range of rotation speed
        speedSlider->setValue(2); // Initial value
        leftPanel->addWidget(speedSlider);

        layout->addLayout(leftPanel);

        label = new QLabel(this);
        layout->addWidget(label);

        // Set the size of the Qt window to match the SDL window size
        this->setFixedSize(QT_WINDOW_WIDTH, QT_WINDOW_HEIGHT);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ImageWidget::updateImage);
        timer->start(16); // Approximately 60 fps

        // Connect the button and slider
        connect(rotateButton, &QPushButton::clicked, this, &ImageWidget::toggleRotation);
        connect(speedSlider, &QSlider::valueChanged, this, &ImageWidget::updateRotationSpeed);
    }

    ~ImageWidget() {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (sdlWindow) SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
    }

    void updateImage() {
        if (!renderer || !texture) return;

        // Render to the SDL window
        SDL_SetRenderTarget(renderer, NULL);  // Set the render target back to the window
        renderContent();  // Render content to the window
        SDL_RenderPresent(renderer);  // Present the window

        // Render off-screen to the texture
        SDL_SetRenderTarget(renderer, texture);  // Set the texture as render target
        renderContent();  // Render content to the texture

        // Capture the rendered texture to a QImage
        QImage image = captureRendererContentToQImage(renderer, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);
        label->setPixmap(QPixmap::fromImage(image));
    }

    // Add member variable to hold the white square texture
    SDL_Texture* whiteTexture = nullptr;

    void renderContent() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set color to black
        SDL_RenderClear(renderer); // Clear the screen with black

        // Draw the green rectangle
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Set color to green
        SDL_Rect greenRect = { 200, 150, 240, 60 }; // Position and size of the green rectangle
        SDL_RenderFillRect(renderer, &greenRect); // Draw the green rectangle

        // If the texture hasn't been created yet, create it
        if (whiteTexture == nullptr) {
            // Create a surface for the white square
            SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 250, 250, 32, SDL_PIXELFORMAT_RGBA8888);
            if (surface == nullptr) {
                qWarning("Failed to create surface for white square. SDL_Error: %s", SDL_GetError());
                return;
            }

            // Fill the surface with a solid white color
            SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

            // Create a texture from the surface
            whiteTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface); // Free the surface after creating the texture
            if (!whiteTexture) {
                qWarning("Failed to create texture from surface. SDL_Error: %s", SDL_GetError());
                return;
            }
        }

        // Static variable to rotate the white square
        static double angle = 0.0;
        angle += rotationSpeed; // Use the current rotation speed

        // Position and size of the white square
        SDL_Rect whiteSquare = { 295, 155, 250, 250 };
        SDL_Point center = { 125, 125 }; // Center of the rectangle for rotation

        // Render the white square with rotation
        SDL_RenderCopyEx(renderer, whiteTexture, nullptr, &whiteSquare, angle, &center, SDL_FLIP_NONE);
    }

    void toggleRotation() {
        // Toggle rotation on or off (for now just a simple start/stop)
        rotationSpeed = (rotationSpeed == 0.0) ? 2.0 : 0.0;
        rotateButton->setText(rotationSpeed == 0.0 ? "Start Rotation" : "Stop Rotation");
    }

    void updateRotationSpeed(int value) {
        // Update rotation speed based on the slider value
        rotationSpeed = value * 0.2; // Scale slider value for a smooth speed range
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ImageWidget window;
    window.setWindowTitle("SDL and Qt Integration with Controls");
    window.show();
    return app.exec();
}
