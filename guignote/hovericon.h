#ifndef HOVERICON_H
#define HOVERICON_H

#include "icon.h"
#include <QPropertyAnimation>
#include <QEnterEvent>
#include <QLabel>

/**
 * @brief Widget interactivo para mostrar un icono con funcionalidad de hover.
 *
 * La clase HoverIcon hereda de Icon y añade una animación de agrandamiento
 * cuando el cursor pasa por encima. Además, muestra una tarjetita con texto.
 */
class HoverIcon : public Icon {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la clase HoverIcon.
     * @param tooltipText Texto que se mostrará en la tarjetita.
     * @param parent Widget padre, por defecto es nullptr.
     *
     * Inicializa el widget heredando las características de Icon y configura la animación.
     */
    explicit HoverIcon(const QString &tooltipText, QWidget *parent = nullptr);

protected:
    /**
     * @brief Evento que se activa cuando el ratón entra en el área del icono.
     * @param event Evento de tipo QEnterEvent.
     *
     * Inicia la animación de agrandamiento del icono y muestra la tarjetita.
     */
    void enterEvent(QEnterEvent *event) override;

    /**
     * @brief Evento que se activa cuando el ratón sale del área del icono.
     * @param event Evento de tipo QEvent.
     *
     * Restaura el tamaño original del icono con una animación y oculta la tarjetita.
     */
    void leaveEvent(QEvent *event) override;

private:
    QPropertyAnimation *animation;
    QRect originalGeometry;
    QLabel *tooltipLabel; ///< Tarjetita con el texto a mostrar
};

#endif // HOVERICON_H
