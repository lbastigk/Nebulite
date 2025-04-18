#include "EditorTab_LevelEditor.h"


LevelEditor::LevelEditor(QWidget *parent)
    : QWidget(parent),
      nebuliteRenderer(true),
      nebuliteShowcaseRenderer(true)
{
    // Init SDL Textures after renderer config
    textureMain = SDL_CreateTexture(
        nebuliteRenderer.getSdlRenderer(),
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        SDL_RENDER_WIDTH,
        SDL_RENDER_HEIGHT
    );

    textureOther = SDL_CreateTexture(
        nebuliteShowcaseRenderer.getSdlRenderer(),
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        SDL_RENDER_WIDTH,
        SDL_RENDER_HEIGHT
    );

    // Create main layout
    mainLayout = new QHBoxLayout(this);  // Set mainLayout as the layout of this widget
    // Don't call setLayout(mainLayout) again here, as it's automatically done in the constructor above.

    // Renderer setup
    nebuliteRenderer.changeWindowSize(SDL_RENDER_WIDTH, SDL_RENDER_HEIGHT);
    nebuliteRenderer.deserializeEnvironment("./Resources/Levels/example.json");
    nebuliteShowcaseRenderer.changeWindowSize(SDL_RENDER_WIDTH, SDL_RENDER_HEIGHT);

    // Initialize widgets
    imageWidget = new ImageWidget(this);
    showcaseImageWidget = new ImageWidget(this);
    testButton = new ButtonWidget("Test", this);
    xSlider = new SliderWidget(0, 3000, 0, true, this);
    ySlider = new SliderWidget(-3000, 0, 0, false, this);
    explorerWidget = new ExplorerWidget(this);

    // Initialize control and output layouts
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->addWidget(showcaseImageWidget, 0, Qt::AlignCenter);
    controlLayout->addWidget(testButton);

    QVBoxLayout *outputLayout = new QVBoxLayout();
    QLabel *mouseStateLabel = new QLabel(this);
    QLabel *cursorPositionLabel = new QLabel(this);
    cursorPositionLabel->setText("Cursor Position: (00000, 00000)");

    ySlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    xSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    imageWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *imgWithSliderY = new QHBoxLayout();
    imgWithSliderY->addWidget(ySlider);
    imgWithSliderY->addWidget(imageWidget, 0, Qt::AlignCenter);

    QVBoxLayout *imgWithSliders = new QVBoxLayout();
    imgWithSliders->addLayout(imgWithSliderY);
    imgWithSliders->addWidget(xSlider);

    outputLayout->addLayout(imgWithSliders);
    outputLayout->addWidget(cursorPositionLabel, 0, Qt::AlignCenter);
    outputLayout->addWidget(mouseStateLabel, 0, Qt::AlignCenter);

    // Add widgets to mainLayout
    mainLayout->addWidget(explorerWidget);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(outputLayout);

    // Update loops
    mainTimer = new QTimer(this);
    othrTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, [this]() { updateMainWindow(); });
    connect(othrTimer, &QTimer::timeout, this, [this]() { updateShowcaseWindow(); });
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
        mouseStateLabel->setText(QString("Mouse Position: (%1, %2)  Mouse State: %3  Wheel delta: %4")
            .arg(ams.currentCursorPos.x())
            .arg(ams.currentCursorPos.y())
            .arg(ams.currentMouseButtonState)
            .arg(wheelDelta)
        ); // Update the label text

        if(wheelDelta > 0 && renderScroller > 0){
            // Zoom in

            //TODO:
            // Get position of mouse, set as new middle point

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
            // Zoom out

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

}


void LevelEditor::updateShowcaseObject(const QString &filePath){
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

            int scalarX = floor((float)SDL_RENDER_HEIGHT / (float)pixX);
            int scalarY = floor((float)SDL_RENDER_WIDTH  / (float)pixY);

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

void LevelEditor::renderContent(Renderer &Renderer, SDL_Texture *texture, float fpsScalar) {
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
    //Renderer.renderFPS(fpsScalar);    // text too small for lower zoom-ins, new approach needed!
    Renderer.showFrame();
}

void LevelEditor::updateShowcaseWindow(){
    updateImage(*showcaseImageWidget,nebuliteShowcaseRenderer,textureOther,  2.0,1.0);
}

void LevelEditor::updateMainWindow(){
    imageWidget->pollMouseState();
    float scalar = (float)nebuliteRenderer.getResX() / (float)SDL_RENDER_WIDTH;
    updateImage(*imageWidget,nebuliteRenderer,textureMain,4.0,scalar);
}

void LevelEditor::updateImage(ImageWidget &img, Renderer &renderer, SDL_Texture *texture, float imageScalar, float rendererScalar) {
    renderContent(renderer,texture,rendererScalar/4);
    img.convertSdlToImage(renderer.getSdlRenderer(), (int)(rendererScalar*(float)SDL_RENDER_WIDTH) , (int)(rendererScalar*(float)SDL_RENDER_HEIGHT), (int)(imageScalar*(float)SDL_RENDER_WIDTH), (int)(imageScalar*(float)SDL_RENDER_HEIGHT));
    img.updateImage();
}
