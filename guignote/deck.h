/**
 * @file deck.h
 * @brief Declaración de la clase Deck, que representa el mazo en la interfaz del juego.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase Deck gestiona el mazo de cartas, su apariencia, el contador de cartas
 * y la carta de triunfo visible.
 */

#ifndef DECK_H
#define DECK_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QDebug>
#include "carta.h"

/**
 * @class Deck
 * @brief Representa el mazo de cartas del juego.
 *
 * Contiene la carta de triunfo, el número de cartas restantes, y su representación visual.
 */
class Deck : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor del mazo.
     * @param triunfo Puntero a la carta de triunfo.
     * @param skin Estilo gráfico de las cartas.
     * @param cardSize Altura deseada de las cartas.
     * @param parent Widget padre.
     * @param token Texto identificador del mazo (opcional).
     */
    explicit Deck(Carta* triunfo, int skin, int cardSize, QWidget *parent = nullptr, QString token = "");

    /** @brief Actualiza la representación visual del mazo. */
    void actualizarVisual();

    /**
     * @brief Establece una nueva carta de triunfo.
     * @param nuevaTriunfo Nueva carta que será la de triunfo.
     */
    void setTriunfo(Carta* nuevaTriunfo);

    /**
     * @brief Establece el número de cartas restantes en el mazo.
     * @param n Nuevo número de cartas.
     */
    void setNum(int n);

    /** @brief Disminuye el número de cartas, simulando que se roba una carta. */
    void cartaRobada();

protected:
    /**
     * @brief Evento que se lanza al redimensionar el widget.
     * @param event Evento de redimensionamiento.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    QString token;           ///< Texto identificador del mazo.
    Carta* triunfo;          ///< Puntero a la carta de triunfo actual.
    int skin;                ///< Estilo gráfico de las cartas.
    int num;                 ///< Número de cartas restantes en el mazo.

    QLabel* fondo;           ///< Etiqueta para la imagen del fondo del mazo.
    QLabel* contador;        ///< Etiqueta que muestra el número de cartas restantes.

    /**
     * @brief Establece la imagen del fondo del mazo.
     * @param pixmap Imagen a establecer.
     * @param alturaDeseada Altura de la imagen.
     */
    void setImagenFondo(const QPixmap &pixmap, int alturaDeseada);
};

#endif // DECK_H
