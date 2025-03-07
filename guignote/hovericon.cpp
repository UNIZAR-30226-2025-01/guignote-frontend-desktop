#include "hovericon.h"
#include <QEvent>

/**
 * @brief Constructor de la clase HoverIcon.
 * @param tooltipText Texto que se mostrará en la tarjetita.
 * @param parent Widget padre, por defecto es nullptr.
 *
 * Llama al constructor de la clase base Icon y configura la animación y la tarjetita.
 */
HoverIcon::HoverIcon(const QString &tooltipText, QWidget *parent)
    : Icon(parent), animation(new QPropertyAnimation(this, "geometry")) {
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    // Crear la tarjetita como una ventana emergente independiente
    tooltipLabel = new QLabel(tooltipText, nullptr);  // Ahora sin `this` como parent
    tooltipLabel->setWindowFlags(Qt::ToolTip);  // Hacer que sea una ventana tipo tooltip
    tooltipLabel->setStyleSheet("background-color: rgba(50, 50, 50, 230); color: white; padding: 5px; border-radius: 10px;");
    tooltipLabel->setAlignment(Qt::AlignCenter);
    tooltipLabel->hide();  // Ocultarla inicialmente
}

/**
 * @brief Evento que se activa cuando el ratón entra en el área del icono.
 * @param event Evento de tipo QEnterEvent.
 *
 * Inicia la animación de agrandamiento del icono y muestra la tarjetita.
 */
void HoverIcon::enterEvent(QEnterEvent *event) {
    originalGeometry = geometry();
    QRect enlargedGeometry = QRect(originalGeometry.x() - 5, originalGeometry.y() - 5,
                                   originalGeometry.width() + 10, originalGeometry.height() + 10);

    animation->setStartValue(originalGeometry);
    animation->setEndValue(enlargedGeometry);
    animation->start();

    // Ajustar tamaño antes de moverla
    tooltipLabel->adjustSize();

    // Calcular la posición para que aparezca justo encima del icono
    int tooltipX = mapToGlobal(QPoint(originalGeometry.width() / 2 - tooltipLabel->width() / 2, 0)).x();
    int tooltipY = mapToGlobal(QPoint(0, -tooltipLabel->height() - 5)).y();

    tooltipLabel->move(tooltipX, tooltipY);
    tooltipLabel->show();
}



/**
 * @brief Evento que se activa cuando el ratón sale del área del icono.
 * @param event Evento de tipo QEvent.
 *
 * Restaura el tamaño original del icono con una animación y oculta la tarjetita.
 */
void HoverIcon::leaveEvent(QEvent *event) {
    animation->setStartValue(geometry());
    animation->setEndValue(originalGeometry);
    animation->start();

    tooltipLabel->hide();  // Ocultar la tarjetita al salir
}

