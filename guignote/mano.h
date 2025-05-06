#ifndef MANO_H
#define MANO_H

#include <QVector>
#include "carta.h"  // ✅ Incluido aquí porque usamos objetos reales (no solo punteros)

#define MARGIN 20         // Margen desde los bordes de la ventana
#define MIN_SPACING 10    // Espaciado mínimo entre cartas

class Mano
{
public:
    Mano(int player_id, int pos);

    void añadirCarta(Carta* carta);
    void eliminarCarta(int index);
    void mostrarMano();
    int player_id;
    bool is_interactive;
    QVector<Carta*> cartas;


private:
    int num_cards;
    int pos;

};

#endif // MANO_H
