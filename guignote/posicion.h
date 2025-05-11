/**
 * @file posicion.h
 * @brief Declaración de la clase Posicion, zona donde se colocan cartas durante la partida.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase Posicion representa un área donde el jugador puede soltar una carta
 * (drag & drop) en la mesa de juego. Gestiona tanto la lógica visual como
 * la interacción con el servidor mediante WebSocket.
 */

#ifndef POSICION_H
#define POSICION_H

#include <QLabel>
#include <QPixmap>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QtWebSockets>
#include "carta.h"

class GameWindow;

/**
 * @class Posicion
 * @brief Zona visual en la que un jugador puede colocar una carta durante su turno.
 *
 * Esta clase hereda de QLabel y permite soltar cartas mediante eventos de drag & drop.
 * Se comunica con el servidor para jugar cartas y refleja su estado en la interfaz.
 */
class Posicion : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la clase Posicion.
     * @param gw Puntero a la ventana de juego.
     * @param parent Widget padre.
     * @param h Altura de la carta a mostrar.
     * @param pos Número o índice de posición en el layout.
     * @param token Token de autenticación.
     * @param ws Puntero al WebSocket para comunicación en tiempo real.
     */
    explicit Posicion(GameWindow *gw, QWidget *parent = nullptr, int h = 100, int pos = 0, QString token = "", QWebSocket *ws = nullptr);

    /** @brief Muestra el fondo base o decorativo de la posición. */
    void mostrarPosicion();

    /** @brief ID del jugador al que pertenece esta posición (si aplica). */
    int player_id;

    /**
     * @brief Establece una carta sobre la posición.
     * @param carta Puntero a la carta que se desea colocar.
     */
    void setCard(Carta* carta);

    /** @brief Elimina la carta actual de la posición (si hay). */
    void removeCard();

    /** @brief Puntero a la carta actualmente colocada en esta posición. */
    Carta* cartaActual;

    /**
     * @brief Activa o desactiva el bloqueo de la posición.
     * @param l Valor booleano que indica si se debe bloquear o no.
     */
    void setLock(bool l);

protected:
    /**
     * @brief Evento al arrastrar un objeto sobre la posición.
     * @param event Evento de arrastre.
     */
    void dragEnterEvent(QDragEnterEvent *event) override;

    /**
     * @brief Evento al soltar un objeto sobre la posición.
     * @param event Evento de soltado.
     */
    void dropEvent(QDropEvent *event) override;

private:
    QWebSocket *ws;           ///< WebSocket para enviar mensajes al servidor.
    QString token;            ///< Token de autenticación del jugador.
    GameWindow *gw;           ///< Puntero a la ventana principal del juego.
    int pos;                  ///< Índice o identificador interno de la posición.
    int alturaCarta;          ///< Altura base de la carta que se mostrará.
    int anchuraCarta;         ///< Ancho calculado de la carta.
    bool Lock;                ///< Indica si la posición está bloqueada para jugar.
    QPixmap fondoBase;        ///< Imagen de fondo para la posición vacía.

    /** @brief Juega la carta colocada en la posición actual (envía mensaje al servidor). */
    void jugarCarta();
};

#endif // POSICION_H
