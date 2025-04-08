#include "mano.h"

Mano::Mano(int player_id, int pos)
{
    this->player_id = player_id;
    num_cards = 0;
    this->pos = pos;
    is_interactive = (pos == 0);
}

void mostrarMiMano(QVector<Carta*> cartas)
{
    if (cartas.isEmpty()) return;

    // Obtenemos el widget padre (asumiendo que todas las cartas tienen el mismo padre)
    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int margenInferior = 20;
    int espacioEntreCartas = 20;

    int altoVentana = contenedor->height();
    int anchoVentana = contenedor->width();

    int anchoCarta = cartas[0]->width();
    int altoCarta = cartas[0]->height();

    int totalAnchoCartas = cartas.size() * anchoCarta + (cartas.size() - 1) * espacioEntreCartas;

    // Punto de inicio centrado horizontalmente
    int xInicial = (anchoVentana - totalAnchoCartas) / 2;
    int y = altoVentana - altoCarta - margenInferior;

    for (int i = 0; i < cartas.size(); ++i) {
        int x = xInicial + i * (anchoCarta + espacioEntreCartas);
        cartas[i]->move(x, y);
        cartas[i]->raise();  // Asegura que estén visibles encima de otros elementos
        cartas[i]->show();   // Asegura que se muestren si estaban ocultas
    }
}


void mostrarMano1(QVector<Carta*> cartas)
{
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int margenDerecho = 20;
    int espacioEntreCartas = 20;

    int altoVentana = contenedor->height();
    int anchoVentana = contenedor->width();

    int anchoCarta = cartas[0]->pixmapOrig.height();  // Rotadas, ahora alto = ancho
    int altoCarta = cartas[0]->pixmapOrig.width();

    int totalAltoCartas = cartas.size() * altoCarta + (cartas.size() - 1) * espacioEntreCartas;

    int x = anchoVentana - anchoCarta - margenDerecho;
    int yInicial = (altoVentana - totalAltoCartas) / 2;

    for (int i = 0; i < cartas.size(); ++i) {
        int y = yInicial + i * (altoCarta + espacioEntreCartas);
        cartas[i]->move(x, y);
        cartas[i]->raise();
        cartas[i]->show();

        // Rotar 270 grados
        QTransform transform;
        transform.rotate(270);

        QPixmap original = cartas[i]->pixmapOrig;
        QPixmap rotado = original.transformed(transform, Qt::SmoothTransformation);

        cartas[i]->setPixmap(rotado);
        cartas[i]->setFixedSize(rotado.size());
    }
}



void mostrarMano2(QVector<Carta*> cartas)
{
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int margenSuperior = 20;
    int espacioEntreCartas = 20;

    int altoVentana = contenedor->height();
    int anchoVentana = contenedor->width();

    int anchoCarta = cartas[0]->width();
    int altoCarta = cartas[0]->height();

    int totalAnchoCartas = cartas.size() * anchoCarta + (cartas.size() - 1) * espacioEntreCartas;

    int xInicial = (anchoVentana - totalAnchoCartas) / 2;
    int y = margenSuperior;

    for (int i = 0; i < cartas.size(); ++i) {
        int x = xInicial + i * (anchoCarta + espacioEntreCartas);
        cartas[i]->move(x, y);
        cartas[i]->raise();
        cartas[i]->show();

        // Rotar 180 grados
        QTransform transform;
        transform.rotate(180);

        QPixmap original = cartas[i]->pixmapOrig;
        QPixmap rotado = original.transformed(transform, Qt::SmoothTransformation);

        cartas[i]->setPixmap(rotado);
        cartas[i]->setFixedSize(rotado.size());
    }
}


void mostrarMano3(QVector<Carta*> cartas)
{
    if (cartas.isEmpty()) return;

    QWidget *contenedor = cartas[0]->parentWidget();
    if (!contenedor) return;

    int margenIzquierdo = 20;
    int espacioEntreCartas = 20;

    int altoVentana = contenedor->height();
    int anchoVentana = contenedor->width();

    // Como están rotadas 90°, el ancho y alto se invierten
    int anchoCarta = cartas[0]->height();
    int altoCarta = cartas[0]->width();

    int totalAltoCartas = cartas.size() * altoCarta + (cartas.size() - 1) * espacioEntreCartas;

    int x = margenIzquierdo;
    int yInicial = (altoVentana - totalAltoCartas) / 2;

    for (int i = 0; i < cartas.size(); ++i) {
        int y = yInicial + i * (altoCarta + espacioEntreCartas);
        cartas[i]->move(x, y);
        cartas[i]->raise();
        cartas[i]->show();

        // Rotar 90 grados
        QTransform transform;
        transform.rotate(90);

        QPixmap original = cartas[i]->pixmapOrig;
        QPixmap rotado = original.transformed(transform, Qt::SmoothTransformation);

        cartas[i]->setPixmap(rotado);
        cartas[i]->setFixedSize(rotado.size());
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
