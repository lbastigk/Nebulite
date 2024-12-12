#include <QApplication>


#include <MainWindow.h>


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.setWindowTitle("SDL and Qt Integration");
    window.show();
    return app.exec();
}
