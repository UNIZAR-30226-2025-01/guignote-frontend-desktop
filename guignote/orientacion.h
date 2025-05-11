/**
 * @file orientacion.h
 * @brief Definición del enumerado Orientacion para representar direcciones en el juego.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * El enumerado Orientacion se utiliza para indicar la orientación o posición
 * relativa de los elementos en la interfaz del juego, como las cartas o los jugadores.
 */

#ifndef ORIENTACION_H
#define ORIENTACION_H

/**
 * @enum Orientacion
 * @brief Enumeración que define las posibles orientaciones en la interfaz del juego.
 *
 * Se utiliza para representar posiciones relativas en la interfaz gráfica,
 * como por ejemplo la orientación de las cartas en pantalla.
 */
enum Orientacion {
    TOP = 0,   ///< Parte superior.
    LEFT,      ///< Lado izquierdo.
    DOWN,      ///< Parte inferior.
    RIGHT      ///< Lado derecho.
};

#endif // ORIENTACION_H
