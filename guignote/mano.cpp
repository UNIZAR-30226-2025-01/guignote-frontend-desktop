/**
 * @file mano.cpp
 * @brief Implementación de la clase Mano.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Define la clase Mano, que gestiona un conjunto de cartas para cada jugador,
 * posicionándolas automáticamente según la orientación (inferior, superior,
 * izquierda o derecha) y permite añadir o eliminar cartas.
 */


#include "mano.h"

/**
 * @brief Constructor de Mano.
 * @param player_id Identificador del jugador dueño de esta mano.
 * @param pos Posición de la mano (0=abajo, 1=arriba, 2=derecha, 3=izquierda).
 *
 * Inicializa el contador de cartas y marca si la mano es interactiva
 * (solo la posición 0 permite interacción directa).
 */

Mano::Mano(int player_id, int pos)
{
    this->player_id = player_id;
    num_cards = 0;
    this->pos = pos;
    is_interactive = (pos == 0);
}

/**
 * @brief Posiciona horizontalmente la mano inferior (posición 0).
 * @param cartas Vector de punteros a las cartas a mostrar.
 *
 * Calcula el espaciado y centra las cartas en el ancho del contenedor,
 * desplazándolas desde la parte baja con un margen fijo.
 */

void mostrarMiMano(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int anchoVentana = contenedor->width();
    int anchoCarta = cartas[0]->width();
    int numCartas = cartas.size();

    int availableWidth = anchoVentana - 2 * MARGIN;
    int totalCardWidth = numCartas * anchoCarta;
    int spacing = 0;

    if (numCartas > 1) {
        spacing = (availableWidth - totalCardWidth) / (numCartas - 1);
        spacing = qMax(spacing, MIN_SPACING);  // Asegurar espaciado mínimo
    }

    int xInicial = (anchoVentana - (totalCardWidth + (numCartas - 1) * spacing)) / 2;
    int y = contenedor->height() - cartas[0]->height() - MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        int x = xInicial + i * (anchoCarta + spacing);
        cartas[i]->move(x, y);
        cartas[i]->raise();
    }
}

/**
 * @brief Posiciona horizontalmente la mano superior (posición 1).
 * @param cartas Vector de punteros a las cartas a mostrar.
 *
 * Centra las cartas en la parte superior del contenedor con un margen fijo.
 */

void mostrarMano2(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int anchoVentana = contenedor->width();
    int anchoCarta = cartas[0]->width();
    int numCartas = cartas.size();

    int availableWidth = anchoVentana - 2 * MARGIN;
    int totalCardWidth = numCartas * anchoCarta;
    int spacing = 0;

    if (numCartas > 1) {
        spacing = (availableWidth - totalCardWidth) / (numCartas - 1);
        spacing = qMax(spacing, MIN_SPACING);
    }

    int xInicial = (anchoVentana - (totalCardWidth + (numCartas - 1) * spacing)) / 2;
    int y = MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        cartas[i]->move(xInicial + i * (anchoCarta + spacing), y);
    }
}

/**
 * @brief Posiciona verticalmente la mano derecha (posición 2).
 * @param cartas Vector de punteros a las cartas a mostrar.
 *
 * Centra las cartas a la derecha del contenedor y ajusta el espaciado vertical.
 */

void mostrarMano1(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int altoVentana = contenedor->height();
    int altoCarta = cartas[0]->height();
    int numCartas = cartas.size();

    int availableHeight = altoVentana - 2 * MARGIN;
    int totalCardHeight = numCartas * altoCarta;
    int spacing = 0;

    if (numCartas > 1) {
        spacing = (availableHeight - totalCardHeight) / (numCartas - 1);
        spacing = qMax(spacing, MIN_SPACING);
    }

    int yInicial = (altoVentana - (totalCardHeight + (numCartas - 1) * spacing)) / 2;
    int x = contenedor->width() - cartas[0]->width() - MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        int y = yInicial + i * (altoCarta + spacing);
        cartas[i]->move(x, y);
    }
}

/**
 * @brief Posiciona verticalmente la mano izquierda (posición 3).
 * @param cartas Vector de punteros a las cartas a mostrar.
 *
 * Centra las cartas a la izquierda del contenedor y ajusta el espaciado vertical.
 */

void mostrarMano3(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int altoVentana = contenedor->height();
    int altoCarta = cartas[0]->height();
    int numCartas = cartas.size();

    int availableHeight = altoVentana - 2 * MARGIN;
    int totalCardHeight = numCartas * altoCarta;
    int spacing = 0;

    if (numCartas > 1) {
        spacing = (availableHeight - totalCardHeight) / (numCartas - 1);
        spacing = qMax(spacing, MIN_SPACING);
    }

    int yInicial = (altoVentana - (totalCardHeight + (numCartas - 1) * spacing)) / 2;
    int x = MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        int y = yInicial + i * (altoCarta + spacing);
        cartas[i]->move(x, y);
    }
}

/**
 * @brief Actualiza la posición de todas las cartas de la mano.
 *
 * Elige la función de posicionamiento adecuada
 * según el valor de `pos` (0–3).
 */

void Mano::mostrarMano()
{
    switch (pos) {
    case 0:
        mostrarMiMano(cartas);
        break;
    case 2:
        mostrarMano1(cartas);
        break;
    case 1:
        mostrarMano2(cartas);
        break;
    case 3:
        mostrarMano3(cartas);
        break;
    }
}

/**
 * @brief Añade una carta a la mano y la posiciona.
 * @param carta Puntero a la carta a añadir.
 *
 * Si es la mano interactiva (posición 0), desbloquea la carta para el usuario,
 * la añade al vector, aumenta el contador y reposiciona todas las cartas.
 */

void Mano::añadirCarta(Carta *carta)
{
    if(pos == 0) {
        carta->setLock(false);
    }
    cartas.append(carta);
    carta->añadirAMano(this, num_cards);
    num_cards++;
    mostrarMano();

}

/**
 * @brief Elimina la carta en la posición dada y reposiciona el resto.
 * @param index Índice de la carta a eliminar en el vector.
 *
 * Disminuye el contador y actualiza la disposición de las cartas.
 */

void Mano::eliminarCarta(int index)
{
    cartas.removeAt(index);
    num_cards--;
    mostrarMano();
}
