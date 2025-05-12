/**
 * @file carta.h
 * @brief Declaración de la clase Carta que representa una carta de juego.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase Carta hereda de QLabel y permite representar visualmente una carta
 * con un palo y un valor, manejando su orientación, posición y eventos del ratón.
 */

#ifndef CARTA_H
#define CARTA_H

#include "orientacion.h"
#include <QString>
#include <QLabel>
#include <QEnterEvent>
#include <QEvent>

/**
 * @class Carta
 * @brief Representación gráfica de una carta de juego.
 *
 * Esta clase extiende QLabel y permite definir las propiedades visuales de una carta,
 * como su palo, valor, posición y orientación. Además, maneja eventos de ratón para
 * interacción del usuario.
 */
class Carta : public QLabel {
    Q_OBJECT

public:
    int skin = 0; ///< Identificador del conjunto gráfico (skin) usado para representar las cartas.

    /**
     * @brief Constructor por defecto.
     * @param parent Widget padre. Opcional.
     */
    Carta(QWidget *parent = nullptr);

    /**
     * @brief Constructor con parámetros de palo y valor.
     * @param palo Palo de la carta (por ejemplo, "corazones", "tréboles").
     * @param valor Valor de la carta (por ejemplo, "A", "7", "K").
     * @param parent Widget padre. Opcional.
     */
    Carta(const QString& palo, const QString& valor, QWidget *parent = nullptr);

    /**
     * @brief Establece el palo y valor de la carta.
     * @param palo Nuevo palo.
     * @param valor Nuevo valor.
     */
    void setPaloValor(const QString& palo, const QString& valor);

    /**
     * @brief Devuelve el palo de la carta.
     * @return QString con el palo.
     */
    QString getPalo() const;

    /**
     * @brief Devuelve el valor de la carta.
     * @return QString con el valor.
     */
    QString getValor() const;

    /**
     * @brief Establece la posición de la carta.
     * @param x Coordenada horizontal.
     * @param y Coordenada vertical.
     */
    void setPosicion(int x, int y);

    void setSkin(int skinId);

    /**
     * @brief Establece la orientación de la carta.
     * @param orientacion Valor de tipo Orientacion (por ejemplo, vertical u horizontal).
     */
    void setOrientacion(Orientacion orientacion);

    bool interactuable = false; ///< Indica si la carta puede ser interactuada por el usuario.

signals:
    /**
     * @brief Señal emitida al hacer doble clic sobre la carta.
     * @param carta Puntero a la carta clicada.
     */
    void cartaDobleClick(Carta* carta);

private:
    /**
     * @brief Carga la imagen de la carta en función del palo, valor y skin.
     */
    void cargarImagen();

    QPixmap img[2]; ///< Imágenes de la carta (cara y dorso).
    QString palo;   ///< Palo de la carta.
    QString valor;  ///< Valor de la carta.
    int posX;       ///< Posición horizontal.
    int posY;       ///< Posición vertical.
    Orientacion orientacion; ///< Orientación de la carta.

protected:
    /**
     * @brief Evento al entrar el cursor sobre la carta.
     * @param event Evento de entrada.
     */
    void enterEvent(QEnterEvent* event) override;

    /**
     * @brief Evento al salir el cursor de la carta.
     * @param event Evento de salida.
     */
    void leaveEvent(QEvent* event) override;

    /**
     * @brief Evento de doble clic del ratón.
     * @param event Evento de tipo QMouseEvent.
     */
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};

#endif // CARTA_H
