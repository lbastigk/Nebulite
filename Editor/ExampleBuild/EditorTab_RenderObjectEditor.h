#ifndef RENDER_OBJECT_EDITOR_H
#define RENDER_OBJECT_EDITOR_H

#include <QWidget>
#include <QHBoxLayout>

class RenderObjectEditor : public QWidget {
    Q_OBJECT
public:
    explicit RenderObjectEditor(QWidget *parent = nullptr);
    QBoxLayout* getLayout() { return mainLayout; }

private:
    QHBoxLayout *mainLayout;
};

#endif // RENDER_OBJECT_EDITOR_H
