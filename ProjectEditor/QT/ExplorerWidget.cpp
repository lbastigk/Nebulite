#include "ExplorerWidget.h"
#include <QVBoxLayout>

ExplorerWidget::ExplorerWidget(QWidget *parent)
    : QWidget(parent), fileModel(new QFileSystemModel(this)), treeView(new QTreeView(this)) {

    // Set up the file model to display the current directory
    fileModel->setRootPath("./");
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    // Configure the tree view
    treeView->setModel(fileModel);
    treeView->setRootIndex(fileModel->index("./"));

    // Set column widths
    treeView->setColumnWidth(0, 300); // Column 0: Name
    treeView->setColumnWidth(1, 100); // Column 1: Size
    treeView->setColumnWidth(2, 100); // Column 2: Type
    treeView->setColumnWidth(3, 100); // Column 3: Date Modified

    // Connect the clicked signal to our custom slot
    connect(treeView, &QTreeView::clicked, this, &ExplorerWidget::onFileClicked);

    // Set up the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(treeView);
    setLayout(layout);
}

void ExplorerWidget::onFileClicked(const QModelIndex &index) {
    // Emit the fileSelected signal with the full path of the clicked file
    QString filePath = fileModel->filePath(index);
    emit fileSelected(filePath);
}
