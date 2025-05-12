/**
 * @file mano.cpp
 * @brief Implementación de la clase Mano, que representa las cartas de un jugador.
 */

#include "mano.h"
#include "carta.h"
#include "estadopartida.h"

/**
 * @brief Constructor de Mano.
 * @param orientacion Orientación de la mano (TOP, LEFT, DOWN, RIGHT).
 * @param estadoPartida Puntero al estado de partida para emitir eventos.
 * @param parent Widget padre.
 */
Mano::Mano(Orientacion orientacion, EstadoPartida* estadoPartida, QWidget* parent)
    : QWidget(parent), orientacion(orientacion), estadoPartida(estadoPartida) {
    zonaJuego = new Carta(this);
    zonaJuego->setOrientacion(orientacion);
}

/**
 * @brief Destructor de Mano. Libera memoria de cartas y zona de juego.
 */
Mano::~Mano() {
    for(int i = 0; i < numCartas; ++i) {
        if(cartas[i]) {
            delete cartas[i];
            cartas[i] = nullptr;
        }
    }
    if(zonaJuego) {
        delete zonaJuego;
        zonaJuego = nullptr;
    }
}

/**
 * @brief Añade una carta a la mano.
 * @param c Puntero a la carta a añadir.
 * @param visible Si la carta debe mostrarse o no.
 */
void Mano::agnadirCarta(Carta* c, bool visible) {
    if(numCartas < 6) {
        cartas[numCartas++] = c;
        c->setParent(this);
        c->setOrientacion(orientacion);
        c->interactuable = visible && (this->orientacion == Orientacion::DOWN);
        if(visible) c->show();
        connect(c, &Carta::cartaDobleClick, estadoPartida, &EstadoPartida::onCartaDobleClick);
    }
}

/**
 * @brief Obtiene una carta por índice.
 * @param i Índice de la carta (0-5).
 * @return Puntero a la carta, o nullptr si no existe.
 */
Carta* Mano::getCarta(int i) const {
    if(i >= 0 && i < numCartas){
        return cartas[i];
    }
    return nullptr;
}

/**
 * @brief Extrae la última carta de la mano.
 * @return Puntero a la carta extraída, o nullptr si no hay cartas.
 */
Carta* Mano::pop() {
    if(numCartas > 0) {
        return extraerCartaEnIndice(numCartas - 1);
    }
    return nullptr;
}

/**
 * @brief Obtiene el número de cartas en la mano.
 * @return Número de cartas.
 */
int Mano::getNumCartas() const {
    return numCartas;
}

/**
 * @brief Extrae una carta concreta según su palo y valor.
 * @param palo Palo de la carta.
 * @param valor Valor de la carta.
 * @return Puntero a la carta extraída, o nullptr si no se encuentra.
 */
Carta* Mano::extraerCarta(const QString& palo, const QString& valor) {
    for(int i = 0; i < numCartas; ++i) {
        if(cartas[i] && cartas[i]->getPalo() == palo && cartas[i]->getValor() == valor) {
            return extraerCartaEnIndice(i);
        }
    }
    return nullptr;
}

/**
 * @brief Extrae una carta en un índice dado.
 * @param indice Índice de la carta.
 * @return Puntero a la carta extraída, o nullptr si índice inválido.
 */
Carta* Mano::extraerCartaEnIndice(int indice) {
    if(indice < 0 || indice >= numCartas)
        return nullptr;
    Carta* extraida = cartas[indice];
    for(int i = indice; i < numCartas - 1; ++i) {
        cartas[i] = cartas[i + 1];
    }
    cartas[numCartas - 1] = nullptr;
    --numCartas;
    return extraida;
}

/**
 * @brief Obtiene la orientación de la mano.
 * @return Valor de orientación.
 */
Orientacion Mano::getOrientacion() const {
    return orientacion;
}

/**
 * @brief Devuelve la carta que representa la carta jugada.
 * @return Puntero a la carta de zona de juego.
 */
Carta* Mano::jugada() {
    return zonaJuego;
}

/**
 * @brief Obtiene la posición de la zona de juego en coordenadas locales.
 * @return Coordenada QPoint de la zona de juego.
 */
QPoint Mano::getZonaDeJuego() const {
    return zonaJuego->pos();
}

/**
 * @brief Actualiza el contenido de la carta jugada.
 * @param palo Nuevo palo de la carta.
 * @param valor Nuevo valor de la carta.
 */
void Mano::actualizarCartaJugada(const QString& palo, const QString& valor, int skinId) {
    zonaJuego->setPaloValor(palo, valor);
    zonaJuego->setOrientacion(this->orientacion);
    zonaJuego->setSkin(skinId);
}

/**
 * @brief Oculta la carta jugada mostrando el reverso.
 */
void Mano::ocultarCartaJugada() {
    zonaJuego->setPaloValor("Back", "");
}

/**
 * @brief Dibuja visualmente la mano en función de la orientación.
 */
void Mano::dibujar() {
    if(numCartas <= 0) return;
    int width  = cartas[0]->width();
    int height = cartas[0]->height();

    switch(orientacion) {
    case(Orientacion::TOP):
        for(int i = 0; i < numCartas; ++i)
            cartas[i]->setPosicion(i * (width + 24), 0);
        zonaJuego->setPosicion((width + 24) * numCartas / 2 - (width / 2), height + 24);
        if(numCartas > 0)
            this->resize((width + 24) * numCartas, height * 2 + 48);
        break;
    case(Orientacion::LEFT):
        for(int i = 0; i < numCartas; ++i)
            cartas[i]->setPosicion(0, i * (height + 24));
        zonaJuego->setPosicion(width + 24, (height + 24) * numCartas / 2 - (height / 2));
        if(numCartas > 0)
            this->resize(width * 2 + 48, (height + 24) * numCartas);
        break;
    case(Orientacion::DOWN):
        for (int i = 0; i < numCartas; ++i)
            cartas[i]->setPosicion(i * (width + 24), height + 24);
        zonaJuego->setPosicion((width + 24) * numCartas / 2 - (width / 2), 0);
        this->resize((width + 24) * numCartas, height * 2 + 48);
        break;
    case(Orientacion::RIGHT):
        for (int i = 0; i < numCartas; ++i)
            cartas[i]->setPosicion(width + 24, i * (height + 24));
        zonaJuego->setPosicion(0, (height + 24) * numCartas / 2 - (height / 2));
        this->resize(width * 2 + 48, (height + 24) * numCartas);
        break;
    }
    this->show();
}
