#ifndef EDITOR_APP
#define EDITOR_APP

#include <QWidget>
#include <QTimer>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>

#include "ImageWidget.h"
#include "ButtonWidget.h"
#include "SliderWidget.h"
#include "ExplorerWidget.h"

#include "Renderer.h"


// Define window size macros for both SDL and Qt
#define SDL_RENDER_WIDTH 160
#define SDL_RENDER_HEIGHT 160

#define QT_WINDOW_WIDTH 2200
#define QT_WINDOW_HEIGHT 1200

#define RENDERER_SCROLLIZE_COUNT 4

class EditorApp : public QWidget {
    Q_OBJECT

public:
    explicit EditorApp(QWidget *parent = nullptr);

    class AppMouseState{
        public:
            QPoint currentCursorPos;
            QPoint lastCursorPos;

            Qt::MouseButtons currentMouseButtonState;
            Qt::MouseButtons lastMouseButtonState;
    };
    AppMouseState ams;

    class Tab{
    public:
        class RenderObjectEditor{
        public:
            RenderObjectEditor();
            QBoxLayout* getLayout(){return mainLayout;};
        private:
            QHBoxLayout *mainLayout;
        }renderObjectEditor;
        class LevelEditor{
        public:
            LevelEditor();

            QBoxLayout* getLayout(){return mainLayout;};

            QHBoxLayout *mainLayout;

            ImageWidget *imageWidget;
            ImageWidget *showcaseImageWidget;
            ExplorerWidget *explorerWidget;

            QTimer *mainTimer;
            QTimer *othrTimer;

            Renderer nebuliteRenderer;
            Renderer nebuliteShowcaseRenderer;

            SDL_Texture *textureMain  = SDL_CreateTexture(
                nebuliteRenderer.getSdlRenderer(), 
                SDL_PIXELFORMAT_RGBA8888, 
                SDL_TEXTUREACCESS_TARGET, 
                SDL_RENDER_WIDTH, 
                SDL_RENDER_HEIGHT
            );
            SDL_Texture *textureOther = SDL_CreateTexture(
                nebuliteShowcaseRenderer.getSdlRenderer(), 
                SDL_PIXELFORMAT_RGBA8888, 
                SDL_TEXTUREACCESS_TARGET,
                SDL_RENDER_WIDTH, 
                SDL_RENDER_HEIGHT
            );

            std::pair<int,int> renderScrollSizes[RENDERER_SCROLLIZE_COUNT] = {
                std::make_pair<int,int>(1 * SDL_RENDER_WIDTH,1 * SDL_RENDER_HEIGHT) , 
                std::make_pair<int,int>(2 * SDL_RENDER_WIDTH,2 * SDL_RENDER_HEIGHT) ,  
                std::make_pair<int,int>(4 * SDL_RENDER_WIDTH,4 * SDL_RENDER_HEIGHT) , 
                std::make_pair<int,int>(8 * SDL_RENDER_WIDTH,8 * SDL_RENDER_HEIGHT) 
            };

            RenderObject selection;
        private:
            void renderContent(Renderer &Renderer, SDL_Texture *texture, float fpsScalar);
            void updateImage(ImageWidget &img,Renderer &renderer,SDL_Texture *texture, float imageScalar, float rendererScalar);
            void updateShowcaseWindow();
            void updateMainWindow();
            void updateShowcaseObject(const QString &filePath);
            RenderObject showcase;
            int renderScroller = 0;
        }levelEditor;
        class MoveRuleSetEditor{
        public:
            MoveRuleSetEditor();
            QBoxLayout* getLayout(){return mainLayout;};
        private:
            QHBoxLayout *mainLayout;
        }moveRuleSetEditor;
    }tab;
private:
    QTabWidget *tabWidget;

    Tab::RenderObjectEditor *tab1;
    Tab::LevelEditor        *tab2;
    Tab::MoveRuleSetEditor  *tab3;
    
private slots:

};

#endif // EDITOR_APP
