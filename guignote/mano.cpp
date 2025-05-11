#include "mano.h"
#include "carta.h"
#include "estadopartida.h"

Mano::Mano(Orientacion orientacion, EstadoPartida* estadoPartida, QWidget* parent)
    : QWidget(parent), orientacion(orientacion), estadoPartida(estadoPartida) {
    zonaJuego = new Carta(this);
    zonaJuego->setOrientacion(orientacion);
}

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

Carta* Mano::getCarta(int i) const {
    if(i >= 0 && i < numCartas){
        return cartas[i];
    }
    return nullptr;
}

Carta* Mano::pop() {
    if(numCartas > 0) {
        return extraerCartaEnIndice(numCartas - 1);
    }
    return nullptr;
}

int Mano::getNumCartas() const {
    return numCartas;
}

Carta* Mano::extraerCarta(const QString& palo, const QString& valor) {
    for(int i = 0; i < numCartas; ++i) {
        if(cartas[i] && cartas[i]->getPalo() == palo && cartas[i]->getValor() == valor) {
            return extraerCartaEnIndice(i);
        }
    }
    return nullptr;
}

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

Orientacion Mano::getOrientacion() const {
    return orientacion;
}

Carta* Mano::jugada() {
    return zonaJuego;
}

QPoint Mano::getZonaDeJuego() const {
    return zonaJuego->pos();
}

void Mano::actualizarCartaJugada(const QString& palo, const QString& valor) {
    zonaJuego->setPaloValor(palo, valor);
    zonaJuego->setOrientacion(this->orientacion);
}

void Mano::ocultarCartaJugada() {
    zonaJuego->setPaloValor("Back", "");
}

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
