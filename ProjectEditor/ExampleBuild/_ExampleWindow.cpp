#include "_ExampleWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "captureRendererContentToQImage.h"  // Helper function to capture SDL output as QImage

_ExampleWindow::_ExampleWindow(QWidget *parent)
    : QWidget(parent),
    nebuliteRenderer(true),  // Passing true for hidden window
    nebuliteShowcaseRenderer(true)  {

    // Initialize widgets
    imageWidget             = new ImageWidget(this);   // Main Image Widget
    showcaseImageWidget     = new ImageWidget(this);
    rotateButton            = new ButtonWidget("Test", this);
    speedSlider             = new SliderWidget(1, 10, 2, this);
    explorerWidget          = new ExplorerWidget(this);

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

    // MAIN Layout
    setLayout(mainLayout);

    // MAIN LOOP
    mainTimer = new QTimer(this);
    othrTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, [this]() { updateImage(*imageWidget,nebuliteRenderer.getSdlRenderer(),textureMain,                   1.0); });
    connect(othrTimer, &QTimer::timeout, this, [this]() { updateImage(*showcaseImageWidget,nebuliteShowcaseRenderer.getSdlRenderer(),textureOther,  0.5); });
    mainTimer->start(16);   //16ms?
    othrTimer->start(160);

    //connections
    connect(explorerWidget, &ExplorerWidget::fileSelected, [](const QString &filePath) {
        qDebug() << "Selected file:" << filePath;
    });

    resize(2200, 1200); // Set the window size
}


void _ExampleWindow::renderContent() {
    RenderObject ro;

    SDL_SetRenderTarget(nebuliteRenderer.getSdlRenderer(), textureMain);
    nebuliteRenderer.append(ro);
    nebuliteRenderer.update();
    nebuliteRenderer.renderFrame();
    nebuliteRenderer.renderFPS();
    nebuliteRenderer.showFrame();
    nebuliteRenderer.purgeObjects();

    SDL_SetRenderTarget(nebuliteShowcaseRenderer.getSdlRenderer(), textureMain);
    nebuliteShowcaseRenderer.update();
    nebuliteShowcaseRenderer.renderFrame();
    nebuliteShowcaseRenderer.renderFPS();
    nebuliteShowcaseRenderer.showFrame();
}

void _ExampleWindow::updateImage(ImageWidget &img, SDL_Renderer *renderer, SDL_Texture *texture, float scalar) {
    SDL_SetRenderTarget(renderer, texture);
    renderContent();
    // Ignoring for now, just using empty QIMAGE for now
    QImage image = captureRendererContentToQImage(renderer, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, (int)(scalar*SDL_WINDOW_WIDTH), (int)(scalar*SDL_WINDOW_HEIGHT));
    img.updateImage(image);
}

