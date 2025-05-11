#include "botonaccion.h"
#include <QPushButton>
#include <QGuiApplication>

BotonAccion::BotonAccion(const QString &texto, std::function<void()> callback, QWidget* parent)
    : QPushButton(texto, parent), callback(callback) {
    setFixedSize(184, 56);
    setStyleSheet(R"(
        QPushButton {
            font-size: 24px;
            color: white;
            background: qlineargradient(
                x1: 0, y1: 0,
                x2: 0, y2: 1,
                stop: 0 #1e1e1e,
                stop: 1 #000000
            );
            border: 2px solid #666;
            border-radius: 12px;
            padding: 10px;
        }

        QPushButton:hover {
            background: qlineargradient(
                x1: 0, y1: 0,
                x2: 0, y2: 1,
                stop: 0 #3a3a3a,
                stop: 1 #222222
            );
            border: 2px solid #999;
        }
    )");
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
}

void BotonAccion::mouseReleaseEvent(QMouseEvent* event) {
    if(callback) {
        callback();
    }
    QPushButton::mouseReleaseEvent(event);
}
