/**
 * @file mano.h
 * @brief Declaración de la clase Mano, que representa la mano de un jugador.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase Mano gestiona las cartas que posee un jugador durante la partida.
 * Permite añadir, eliminar y mostrar cartas, y mantiene información sobre
 * la posición y el jugador asociado.
 */

#ifndef MANO_H
#define MANO_H

#include <QVector>
#include "carta.h"  // ✅ Incluido porque se trabaja directamente con objetos Carta

#define MARGIN 20         ///< Margen desde los bordes de la ventana.
#define MIN_SPACING 10    ///< Espaciado mínimo entre cartas.

/**
 * @class Mano
 * @brief Representa la mano de cartas de un jugador.
 *
 * Permite almacenar, modificar y mostrar las cartas que tiene un jugador en su mano.
 */
class Mano
{
public:
    /**
     * @brief Constructor de la clase Mano.
     * @param player_id Identificador del jugador asociado.
     * @param pos Posición visual o lógica en la interfaz de juego.
     */
    Mano(int player_id, int pos);

    /**
     * @brief Añade una carta a la mano del jugador.
     * @param carta Puntero a la carta a añadir.
     */
    void añadirCarta(Carta* carta);

    /**
     * @brief Elimina una carta de la mano por índice.
     * @param index Índice de la carta a eliminar.
     */
    void eliminarCarta(int index);

    /**
     * @brief Muestra visualmente las cartas de la mano.
     */
    void mostrarMano();

    int player_id;             ///< Identificador del jugador dueño de esta mano.
    bool is_interactive;       ///< True si el jugador puede interactuar con las cartas.
    QVector<Carta*> cartas;    ///< Cartas actualmente en la mano.

private:
    int num_cards;             ///< Número de cartas actuales.
    int pos;                   ///< Posición relativa o lógica de la mano.
};

#endif // MANO_H
