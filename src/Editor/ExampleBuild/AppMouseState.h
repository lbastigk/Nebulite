#ifndef APP_MOUSE_STATE
#define APP_MOUSE_STATE

#include <QWidget>
#include <QTabWidget>
#include <QPoint>
#include <QMouseEvent>


struct AppMouseState{
        QPoint currentCursorPos;
        QPoint lastCursorPos;

        Qt::MouseButtons currentMouseButtonState;
        Qt::MouseButtons lastMouseButtonState;
};

#endif // APP_MOUSE_STATE