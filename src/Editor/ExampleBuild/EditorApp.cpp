#include "EditorApp.h"

#define QT_WINDOW_WIDTH 1920
#define QT_WINDOW_HEIGHT 1080

EditorApp::EditorApp(QWidget *parent)
    : QWidget(parent)
{
    tabWidget = new QTabWidget(this);

    tab1 = new RenderObjectEditor(this);
    tab2 = new LevelEditor(this);
    tab3 = new MoveRuleSetEditor(this);

    tabWidget->addTab(tab1, "Render Object Editor");
    tabWidget->addTab(tab2, "Level Editor");
    tabWidget->addTab(tab3, "Move Rule Set Editor");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);
    setWindowTitle("Nebulite Editor");
    resize(QT_WINDOW_WIDTH, QT_WINDOW_HEIGHT);
}

