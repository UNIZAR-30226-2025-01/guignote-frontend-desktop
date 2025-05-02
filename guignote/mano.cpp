#include "mano.h"

Mano::Mano(int player_id, int pos)
{
    this->player_id = player_id;
    num_cards = 0;
    this->pos = pos;
    is_interactive = (pos == 0);
}

// Función mostrarMiMano (posición 0 - mano inferior)
void mostrarMiMano(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int anchoVentana = contenedor->width();
    int anchoCarta = cartas[0]->width();
    int numCartas = cartas.size();

    // Calcular ancho total requerido
    int totalAncho = (numCartas * anchoCarta) + ((numCartas - 1) * MIN_SPACING);

    // Posición inicial para centrar
    int xInicial = (anchoVentana - totalAncho) / 2;
    int y = contenedor->height() - cartas[0]->height() - MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        cartas[i]->move(xInicial + i * (anchoCarta + MIN_SPACING), y);
        cartas[i]->raise();
    }
}

// Función mostrarMano1 (posición 2 - derecha vertical)
void mostrarMano1(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int altoVentana = contenedor->height();
    int altoCarta = cartas[0]->getOriginalPixmap().width();
    int numCartas = cartas.size();

    // Calcular alto total requerido
    int totalAlto = (numCartas * altoCarta) + ((numCartas - 1) * MIN_SPACING);

    int yInicial = (altoVentana - totalAlto) / 2;
    int x = contenedor->width() - cartas[0]->height() - MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        int y = yInicial + i * (altoCarta + MIN_SPACING);
        cartas[i]->move(x, y);
    }
}

// Función mostrarMano2 (posición 1 - arriba horizontal)
void mostrarMano2(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int anchoVentana = contenedor->width();
    int anchoCarta = cartas[0]->width();
    int numCartas = cartas.size();

    int totalAncho = (numCartas * anchoCarta) + ((numCartas - 1) * MIN_SPACING);
    int xInicial = (anchoVentana - totalAncho) / 2;
    int y = MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        cartas[i]->move(xInicial + i * (anchoCarta + MIN_SPACING), y);
    }
}

// Función mostrarMano3 (posición 3 - izquierda vertical)
void mostrarMano3(QVector<Carta*> cartas) {
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int altoVentana = contenedor->height();
    int altoCarta = cartas[0]->getOriginalPixmap().width();
    int numCartas = cartas.size();

    int totalAlto = (numCartas * altoCarta) + ((numCartas - 1) * MIN_SPACING);
    int yInicial = (altoVentana - totalAlto) / 2;
    int x = MARGIN;

    for(int i = 0; i < numCartas; ++i) {
        int y = yInicial + i * (altoCarta + MIN_SPACING);
        cartas[i]->move(x, y);
    }
}

// Crear funciones Auxiliares
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

void Mano::eliminarCarta(int index)
{
    cartas.removeAt(index);
    num_cards--;
    mostrarMano();
}
