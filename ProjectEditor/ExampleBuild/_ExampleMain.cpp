#include <QApplication>


#include <_ExampleWindow.h>


int main(int argc, char *argv[]) {
    // Redirect std::cerr to a file
    freopen("error_log.txt", "w", stderr);

    QApplication app(argc, argv);
    _ExampleWindow window;
    window.setWindowTitle("SDL and Qt Integration");
    window.show();
    return app.exec();
}
