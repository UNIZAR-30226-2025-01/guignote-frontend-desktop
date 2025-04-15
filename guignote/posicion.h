#ifndef POSICION_H
#define POSICION_H

#include <QLabel>
#include <QPixmap>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include "carta.h"

class Posicion : public QLabel
{
    Q_OBJECT

public:
    explicit Posicion(GameWindow *gw, QWidget *parent = nullptr, int h = 100, int pos = 0);
    void mostrarPosicion();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    GameWindow *gw;
    int pos;
    int alturaCarta;
    int anchuraCarta;
    bool Lock;
    Carta* cartaActual;

    QPixmap fondoBase;
};

#endif // POSICION_H
