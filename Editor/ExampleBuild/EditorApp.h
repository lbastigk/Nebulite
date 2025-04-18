#ifndef EDITOR_APP
#define EDITOR_APP

#include <QWidget>
#include <QTabWidget>
#include <QPoint>
#include <QMouseEvent>

#include "EditorTab_LevelEditor.h"
#include "EditorTab_RenderObjectEditor.h"
#include "EditorTab_MoveRuleSetEditor.h"

class EditorApp : public QWidget {
    Q_OBJECT

public:
    explicit EditorApp(QWidget *parent = nullptr);

    class AppMouseState {
    public:
        QPoint currentCursorPos;
        QPoint lastCursorPos;

        Qt::MouseButtons currentMouseButtonState;
        Qt::MouseButtons lastMouseButtonState;
    };
    AppMouseState ams;

private:
    QTabWidget *tabWidget;

    LevelEditor *tab2;
    RenderObjectEditor *tab1;
    MoveRuleSetEditor *tab3;
};

#endif // EDITOR_APP
