/**
 * @file botonaccion.cpp
 * @brief Implementación de la clase BotonAccion.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la implementación de los métodos de la clase BotonAccion, que
 * extiende QPushButton para ejecutar una acción personalizada al hacer clic.
 */

#include "botonaccion.h"
#include <QPushButton>
#include <QGuiApplication>

/**
 * @brief Constructor de la clase BotonAccion.
 *
 * Inicializa un botón con el texto proporcionado, aplicando estilos visuales
 * y asignando una función callback que se ejecutará al soltar el botón.
 *
 * @param texto Texto que se mostrará en el botón.
 * @param callback Función a ejecutar al hacer clic. Puede ser nula.
 * @param parent Widget padre del botón. Opcional.
 */
BotonAccion::BotonAccion(const QString &texto, std::function<void()> callback, QWidget* parent)
    : QPushButton(texto, parent), callback(callback)
{
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

/**
 * @brief Evento que se ejecuta al soltar el botón del ratón.
 *
 * Si se ha proporcionado una función de callback, se ejecuta justo antes
 * de invocar el comportamiento por defecto del botón.
 *
 * @param event Evento de tipo QMouseEvent.
 */
void BotonAccion::mouseReleaseEvent(QMouseEvent* event) {
    if (callback) {
        callback();
    }
    QPushButton::mouseReleaseEvent(event);
}
