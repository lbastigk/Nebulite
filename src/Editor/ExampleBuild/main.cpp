#include <QApplication>


#include <EditorApp.h>


int main(int argc, char *argv[]) {
    // Redirect std::cerr to a file
    freopen("error_log.txt", "w", stderr);

    QApplication app(argc, argv);
    EditorApp window;
    window.setWindowTitle("SDL and Qt Integration");
    window.show();
    return app.exec();
}
