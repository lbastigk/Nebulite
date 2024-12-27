#include "_ExampleWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

_ExampleWindow::_ExampleWindow(QWidget *parent)
    : QWidget(parent),
    nebuliteRenderer(true),  // Passing true for hidden window
    nebuliteShowcaseRenderer(true)  {

    nebuliteRenderer.deserializeEnvironment("./Resources/Levels/example.json", nebuliteRenderer.getResX(), nebuliteRenderer.getResY(), nebuliteRenderer.getThreadSize());

    std::cerr << "Position: x" << nebuliteRenderer.getPosX() << " y" << nebuliteRenderer.getPosY() << std::endl;

    //-------------------------------------------------------------------------
    // Initialize widgets
    imageWidget             = new ImageWidget(this);   // Main Image Widget
    showcaseImageWidget     = new ImageWidget(this);
    testButton            = new ButtonWidget("Test", this);
    xSlider                 = new SliderWidget(-1000, 1000, 0, this);
    explorerWidget          = new ExplorerWidget(this);

    //-------------------------------------------------------------------------
    // setup explorer
    explorerWidget->changePath("./Resources/");

    //-------------------------------------------------------------------------
    // Create control layout (vertical)
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->addWidget(showcaseImageWidget,0,Qt::AlignCenter);  // SDL placeholder on top
    controlLayout->addWidget(testButton);
    controlLayout->addWidget(xSlider);

    //-------------------------------------------------------------------------
    // Create output layout (vertical)
    QVBoxLayout *outputLayout = new QVBoxLayout();

    // Make sure cursorPositionLabel is valid throughout the object's lifetime
    QLabel *cursorPositionLabel = new QLabel(this);
    cursorPositionLabel->setText("Cursor Position: (0, 0)");

    QTimer *labelUpdateTimer = new QTimer(this);
    connect(labelUpdateTimer, &QTimer::timeout, this, [this, cursorPositionLabel]() {
        if (!this || !cursorPositionLabel) {
            std::cerr << "Error: Object or label is null!";
            return;
        }

        imageWidget->pollMousePosition();
        QPoint csr = imageWidget->getCursorPos();
        cursorPositionLabel->setText(QString("Cursor Position: (%1, %2)").arg(csr.x()).arg(csr.y())); // Update the label text
    });

    labelUpdateTimer->start(16); // Update at ~60Hz

    // Add widgets to layout
    outputLayout->addWidget(imageWidget, 0, Qt::AlignCenter);
    outputLayout->addWidget(cursorPositionLabel, 0, Qt::AlignCenter);
    
    //-------------------------------------------------------------------------
    // Create main layout (horizontal)
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(explorerWidget);    // Explorer on the left
    mainLayout->addLayout(controlLayout);     // Controls in the middle
    mainLayout->addLayout(outputLayout);

    setLayout(mainLayout);

    //-------------------------------------------------------------------------
    // Update loops
    mainTimer = new QTimer(this);
    othrTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, [this]() { updateMainWindow(); });    // using SDL texture from _ExampleWindow class
    connect(othrTimer, &QTimer::timeout, this, [this]() { updateShowcaseWindow(); });    // using SDL texture from _ExampleWindow class
    mainTimer->start(16);
    othrTimer->start(16);

    //-------------------------------------------------------------------------
    // Other connections
    connect(explorerWidget, &ExplorerWidget::fileSelected, this, [this](const QString &filePath) {updateShowcaseObject(filePath);});

    connect(xSlider, &SliderWidget::valueChanged, [this](int value){
        nebuliteRenderer.updatePosition(value,0,false);
        std::cerr << "Position: x" << nebuliteRenderer.getPosX() << " y" << nebuliteRenderer.getPosY() << std::endl;
    });

    //-------------------------------------------------------------------------
    // App size
    resize(2200, 1200); // Set the window size
}

void _ExampleWindow::updateShowcaseObject(const QString &filePath){
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

void _ExampleWindow::updateShowcaseWindow(){
    updateImage(*showcaseImageWidget,nebuliteShowcaseRenderer,textureOther,  0.5);
}

void _ExampleWindow::updateMainWindow(){
    imageWidget->pollMousePosition();
    updateImage(*imageWidget,nebuliteRenderer,textureMain,                   1.0);
}

void _ExampleWindow::updateImage(ImageWidget &img, Renderer &renderer, SDL_Texture *texture, float scalar) {
    renderContent(renderer,texture);
    img.convertSdlToImage(renderer.getSdlRenderer(), SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, (int)(scalar*SDL_WINDOW_WIDTH), (int)(scalar*SDL_WINDOW_HEIGHT));
    img.updateImage();
}

