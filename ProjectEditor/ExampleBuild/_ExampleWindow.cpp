#include "_ExampleWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

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

    // setup explorer
    explorerWidget->changePath("./Resources/");

    // Create control layout (vertical)
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->addWidget(showcaseImageWidget);  // SDL placeholder on top
    controlLayout->addWidget(rotateButton);
    controlLayout->addWidget(speedSlider);

    // Create main layout (horizontal)
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(explorerWidget);    // Explorer on the left
    mainLayout->addLayout(controlLayout);     // Controls in the middle
    mainLayout->addWidget(imageWidget);       // Main image on the right

    // MAIN Layout
    setLayout(mainLayout);

    // MAIN LOOP
    mainTimer = new QTimer(this);
    othrTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, [this]() { updateImage(*imageWidget,nebuliteRenderer,textureMain,                   1.0); });    // using SDL texture from _ExampleWindow class
    connect(othrTimer, &QTimer::timeout, this, [this]() { updateImage(*showcaseImageWidget,nebuliteShowcaseRenderer,textureOther,  0.5); });    // using SDL texture from _ExampleWindow class
    mainTimer->start(16);   //16ms?
    othrTimer->start(16);

    //connections
    connect(explorerWidget, &ExplorerWidget::fileSelected, this, [this](const QString &filePath) {updateShowcase(filePath);});

    resize(2200, 1200); // Set the window size
}

void _ExampleWindow::updateShowcase(const QString &filePath){
    std::cerr << "New File: " << filePath.toStdString() << std::endl;

    if(filePath.toStdString().ends_with(".json")){
        rapidjson::Document doc = JSONHandler::deserialize(filePath.toStdString());
        if (doc.IsObject()){
            RenderObject roc;
            roc.deserialize(JSONHandler::serialize(doc));

            int pixX = roc.valueGet<int>(namenKonvention.renderObject.pixelSizeX);
            int pixY = roc.valueGet<int>(namenKonvention.renderObject.pixelSizeY);

            if (pixX < 0){pixX = 1;}
            if (pixY < 0){pixY = 1;}

            int scalarX = floor((float)SDL_WINDOW_HEIGHT / (float)pixX);
            int scalarY = floor((float)SDL_WINDOW_WIDTH  / (float)pixY);

            int scalar;
            if(scalarX < scalarY){
                scalar = scalarX;
            }
            else{
                scalar = scalarY;
            }

            roc.valueSet(namenKonvention.renderObject.positionX,0);
            roc.valueSet(namenKonvention.renderObject.positionY,0);

            roc.valueSet(namenKonvention.renderObject.pixelSizeX,scalar*pixX);
            roc.valueSet(namenKonvention.renderObject.pixelSizeY,scalar*pixY);

            nebuliteShowcaseRenderer.purgeObjects();
            nebuliteShowcaseRenderer.append(roc);
        }
    }

    
}

void _ExampleWindow::renderContent(Renderer &Renderer, SDL_Texture *texture) {
    if (!Renderer.getSdlRenderer()) {
        std::cerr << "Error: SDL Renderer is null!\n";
        return;
    }
    if (!texture) {
        std::cerr << "Error: SDL Texture is null!\n";
        return;
    }


    SDL_SetRenderTarget(Renderer.getSdlRenderer(), texture);    //< Crashes when calling updateImage of showcaseImageWidget
    Renderer.update();
    Renderer.renderFrame();
    //Renderer.renderFPS();
    Renderer.showFrame();
}

void _ExampleWindow::updateImage(ImageWidget &img, Renderer &renderer, SDL_Texture *texture, float scalar) {
    renderContent(renderer,texture);
    img.convertSdlToImage(renderer.getSdlRenderer(), SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, (int)(scalar*SDL_WINDOW_WIDTH), (int)(scalar*SDL_WINDOW_HEIGHT));
    img.updateImage();
}

