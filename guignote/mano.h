#ifndef MANO_H
#define MANO_H

#include "orientacion.h"
#include "carta.h"
#include <QWidget>

class EstadoPartida;

class Mano : public QWidget {
    Q_OBJECT
public:

    Mano(Orientacion orientacion, EstadoPartida* estadoPartida, QWidget* parent);
    ~Mano();

    // Cartas en la mano
    void agnadirCarta(Carta* c, bool visible = true);
    Carta* getCarta(int i) const;
    Carta* pop();
    int getNumCartas() const;
    Carta* extraerCarta(const QString& palo, const QString& valor);
    Carta* extraerCartaEnIndice(int indice);

    // Carta jugada
    Carta* jugada();
    QPoint getZonaDeJuego() const;
    void actualizarCartaJugada(const QString& palo, const QString& valor);
    void ocultarCartaJugada();

    // GUI
    Orientacion getOrientacion() const;
    void dibujar();

private:
    Carta* zonaJuego;
    Carta* cartas[6];
    int numCartas = 0;
    const Orientacion orientacion;
    EstadoPartida* estadoPartida;
};

#endif // MANO_H
