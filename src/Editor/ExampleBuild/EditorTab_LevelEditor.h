#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include <QWidget>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>

#include "ImageWidget.h"
#include "ButtonWidget.h"
#include "SliderWidget.h"
#include "ExplorerWidget.h"
#include "Renderer.h"
#include "AppMouseState.h"

// SDL defines
#define SDL_RENDER_WIDTH 160
#define SDL_RENDER_HEIGHT 160
#define RENDERER_SCROLLIZE_COUNT 4

class LevelEditor : public QWidget {
    Q_OBJECT
public:
    explicit LevelEditor(QWidget *parent = nullptr);
    QBoxLayout* getLayout() { return mainLayout; }

private:
    AppMouseState ams; 

    QHBoxLayout *mainLayout;

    ImageWidget *imageWidget;
    ImageWidget *showcaseImageWidget;
    ExplorerWidget *explorerWidget;

    QTimer *mainTimer;
    QTimer *othrTimer;

    Renderer nebuliteRenderer;
    Renderer nebuliteShowcaseRenderer;

    SDL_Texture *textureMain;
    SDL_Texture *textureOther;

    SliderWidget *xSlider;
    SliderWidget *ySlider;

    ButtonWidget *testButton;

    std::pair<int,int> renderScrollSizes[RENDERER_SCROLLIZE_COUNT] = {
        {1 * SDL_RENDER_WIDTH, 1 * SDL_RENDER_HEIGHT},
        {2 * SDL_RENDER_WIDTH, 2 * SDL_RENDER_HEIGHT},
        {4 * SDL_RENDER_WIDTH, 4 * SDL_RENDER_HEIGHT},
        {8 * SDL_RENDER_WIDTH, 8 * SDL_RENDER_HEIGHT}
    };

    RenderObject selection;
    RenderObject showcase;
    int renderScroller = 0;

    // Functions
    void renderContent(Renderer &Renderer, SDL_Texture *texture, float fpsScalar);
    void updateImage(ImageWidget &img, Renderer &renderer, SDL_Texture *texture, float imageScalar, float rendererScalar);
    void updateShowcaseWindow();
    void updateMainWindow();
    void updateShowcaseObject(const QString &filePath);
};

#endif // LEVEL_EDITOR_H

