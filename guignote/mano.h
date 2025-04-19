#ifndef MANO_H
#define MANO_H

#include <QVector>
#include "carta.h"  // ✅ Incluido aquí porque usamos objetos reales (no solo punteros)

class Mano
{
public:
    Mano(int player_id, int pos);

    void añadirCarta(Carta* carta);
    void eliminarCarta(int index);
    void mostrarMano();
    int player_id;
    bool is_interactive;

private:
    int num_cards;
    int pos;
    QVector<Carta*> cartas;
};

#endif // MANO_H
