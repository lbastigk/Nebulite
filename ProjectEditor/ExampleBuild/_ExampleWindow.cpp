#include "_ExampleWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

_ExampleWindow::_ExampleWindow(QWidget *parent)
    : QWidget(parent),
    nebuliteRenderer(true),  // Passing true for hidden window
    nebuliteShowcaseRenderer(true)  {

    nebuliteRenderer.changeWindowSize(SDL_WINDOW_WIDTH,SDL_WINDOW_HEIGHT);
    nebuliteRenderer.deserializeEnvironment("./Resources/Levels/example.json", nebuliteRenderer.getResX(), nebuliteRenderer.getResY(), nebuliteRenderer.getThreadSize());

    nebuliteShowcaseRenderer.changeWindowSize(SDL_WINDOW_WIDTH,SDL_WINDOW_HEIGHT);

    //-------------------------------------------------------------------------
    // Initialize widgets
    imageWidget             = new ImageWidget(this);   // Main Image Widget
    showcaseImageWidget     = new ImageWidget(this);
    testButton              = new ButtonWidget("Test", this);
    xSlider                 = new SliderWidget(-1000, 1000, 0,true,  this);
    ySlider                 = new SliderWidget(-1000, 1000, 0,false, this);
    explorerWidget          = new ExplorerWidget(this);

    //-------------------------------------------------------------------------
    // setup explorer
    explorerWidget->changePath("./Resources/");

    //-------------------------------------------------------------------------
    // Create control layout (vertical)
    QVBoxLayout *controlLayout = new QVBoxLayout();

    // Showcase
    controlLayout->addWidget(showcaseImageWidget,0,Qt::AlignCenter);  // SDL placeholder on top

    // Test Button
    controlLayout->addWidget(testButton);

    //-------------------------------------------------------------------------
    // Create output layout (vertical)
    QVBoxLayout *outputLayout = new QVBoxLayout();

    // Mouse state label
    QLabel *mouseStateLabel = new QLabel(this);

    // Make sure cursorPositionLabel is valid throughout the object's lifetime
    QLabel *cursorPositionLabel = new QLabel(this);
    cursorPositionLabel->setText("Cursor Position: (00000, 00000)");

    // Ensure the sliders do not stretch
    ySlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding); // Vertical slider fixed width
    xSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Horizontal slider fixed height

    // Set the image widget to expand and fill the available space
    imageWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create Image Widget with x and y sliders
    QHBoxLayout *imgWithSliderY = new QHBoxLayout();
    imgWithSliderY->addWidget(ySlider);
    imgWithSliderY->addWidget(imageWidget);

    QVBoxLayout *imgWithSliders = new QVBoxLayout();
    imgWithSliders->addLayout(imgWithSliderY);
    imgWithSliders->addWidget(xSlider);

    // Add widgets to layout
    outputLayout->addLayout(imgWithSliders);
    outputLayout->addWidget(cursorPositionLabel, 0, Qt::AlignCenter);
    outputLayout->addWidget(mouseStateLabel,     0, Qt::AlignCenter);
    
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

    // File path info
    connect(explorerWidget, &ExplorerWidget::fileSelected, this, [this](const QString &filePath) {updateShowcaseObject(filePath);});

    // X and Y slider
    connect(xSlider, &SliderWidget::valueChanged, [this](int value){
        nebuliteRenderer.updatePosition(value, nebuliteRenderer.getPosY(), false);
    });
    connect(ySlider, &SliderWidget::valueChanged, [this](int value) {
        nebuliteRenderer.updatePosition(nebuliteRenderer.getPosX(), -value, false);
    });

    // X Y position Label
    QTimer *labelUpdateTimer = new QTimer(this);
    connect(labelUpdateTimer, &QTimer::timeout, this, [this, cursorPositionLabel]() {
        if (!this || !cursorPositionLabel) {
            std::cerr << "Error: Object or label is null!";
            return;
        }
        // Polling
        imageWidget->pollMouseState();

        ams.lastCursorPos = ams.currentCursorPos;
        ams.lastMouseButtonState = ams.currentMouseButtonState;

        ams.currentCursorPos = imageWidget->getCursorPos();
        ams.currentMouseButtonState = imageWidget->getMouseState();
        
        // Updating label
        cursorPositionLabel->setText(
            QString("Pos: (%1 %2)  Res: (%3 %4)  Tile: (%5 %6)")
            .arg(nebuliteRenderer.getPosX())
            .arg(nebuliteRenderer.getPosY())
            .arg(nebuliteRenderer.getResX())
            .arg(nebuliteRenderer.getResY())
            .arg(nebuliteRenderer.getTileXpos())
            .arg(nebuliteRenderer.getTileYpos())
        );
    });
    labelUpdateTimer->start(16); // Update at ~60Hz

    // Mouse state label
    QTimer *stateUpdateTimer = new QTimer(this);
    connect(stateUpdateTimer, &QTimer::timeout, this, [this, mouseStateLabel]() {       
        int wheelDelta = imageWidget->getWheelDelta();

        // Updating label
        mouseStateLabel->setText(QString("Mouse State: %1  Wheel delta: %2").arg(ams.currentMouseButtonState).arg(wheelDelta)); // Update the label text

        if(wheelDelta > 0 && renderScroller > 0){
            renderScroller--;
            nebuliteRenderer.changeWindowSize(renderScrollSizes[renderScroller].first,renderScrollSizes[renderScroller].second);

            textureMain  = SDL_CreateTexture(
                nebuliteRenderer.getSdlRenderer(), 
                SDL_PIXELFORMAT_RGBA8888, 
                SDL_TEXTUREACCESS_TARGET, 
                nebuliteRenderer.getResX(), 
                nebuliteRenderer.getResY()
            );
        }
        if(wheelDelta < 0 && renderScroller < (RENDERER_SCROLLIZE_COUNT - 1)){
            renderScroller++;
            nebuliteRenderer.changeWindowSize(renderScrollSizes[renderScroller].first,renderScrollSizes[renderScroller].second);

            textureMain  = SDL_CreateTexture(
                nebuliteRenderer.getSdlRenderer(), 
                SDL_PIXELFORMAT_RGBA8888, 
                SDL_TEXTUREACCESS_TARGET, 
                nebuliteRenderer.getResX(), 
                nebuliteRenderer.getResY()
            );
        }
    });
    stateUpdateTimer->start(16); // Update at ~60Hz


    //-------------------------------------------------------------------------
    // App size
    resize(QT_WINDOW_WIDTH, QT_WINDOW_HEIGHT); // Set the window size
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

void _ExampleWindow::renderContent(Renderer &Renderer, SDL_Texture *texture, float fpsScalar) {
    if (!Renderer.getSdlRenderer()) {
        std::cerr << "Error: SDL Renderer is null!\n";
        return;
    }
    if (!texture) {
        std::cerr << "Error: SDL Texture is null!\n";
        return;
    }


    SDL_SetRenderTarget(Renderer.getSdlRenderer(), texture);
    Renderer.update_withThreads();
    Renderer.renderFrame();
    Renderer.renderFPS(fpsScalar);
    Renderer.showFrame();
}

void _ExampleWindow::updateShowcaseWindow(){
    updateImage(*showcaseImageWidget,nebuliteShowcaseRenderer,textureOther,  0.5,1.0);
}

void _ExampleWindow::updateMainWindow(){
    imageWidget->pollMouseState();
    float scalar = (float)nebuliteRenderer.getResX() / (float)SDL_WINDOW_WIDTH;
    updateImage(*imageWidget,nebuliteRenderer,textureMain,1.0,scalar);
}

void _ExampleWindow::updateImage(ImageWidget &img, Renderer &renderer, SDL_Texture *texture, float imageScalar, float rendererScalar) {
    renderContent(renderer,texture,rendererScalar);
    img.convertSdlToImage(renderer.getSdlRenderer(), (int)(rendererScalar*(float)SDL_WINDOW_WIDTH) , (int)(rendererScalar*(float)SDL_WINDOW_HEIGHT), (int)(imageScalar*(float)SDL_WINDOW_WIDTH), (int)(imageScalar*(float)SDL_WINDOW_HEIGHT));
    //img.readTextureToImage(texture,(int)(rendererScalar*(float)SDL_WINDOW_WIDTH) , (int)(rendererScalar*(float)SDL_WINDOW_HEIGHT), (int)(imageScalar*(float)SDL_WINDOW_WIDTH), (int)(imageScalar*(float)SDL_WINDOW_HEIGHT));
    img.updateImage();
}

