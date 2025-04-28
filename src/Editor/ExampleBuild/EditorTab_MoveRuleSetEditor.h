#ifndef MOVE_RULE_SET_EDITOR_H
#define MOVE_RULE_SET_EDITOR_H

#include <QWidget>
#include <QHBoxLayout>

class MoveRuleSetEditor : public QWidget {
    Q_OBJECT
public:
    explicit MoveRuleSetEditor(QWidget *parent = nullptr);
    QBoxLayout* getLayout() { return mainLayout; }

private:
    QHBoxLayout *mainLayout;
};

#endif // MOVE_RULE_SET_EDITOR_H
