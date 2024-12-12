#ifndef EXPLORERWIDGET_H
#define EXPLORERWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>

class ExplorerWidget : public QWidget {
    Q_OBJECT

    QFileSystemModel *fileModel;
    QTreeView *treeView;

public:
    explicit ExplorerWidget(QWidget *parent = nullptr);

signals:
    void fileSelected(const QString &filePath);

private slots:
    void onFileClicked(const QModelIndex &index);
};

#endif // EXPLORERWIDGET_H
