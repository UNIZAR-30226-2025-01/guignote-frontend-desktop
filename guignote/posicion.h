#ifndef POSICION_H
#define POSICION_H

#include <QLabel>
#include <QPixmap>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QtWebSockets>
#include "carta.h"

class Posicion : public QLabel
{
    Q_OBJECT

public:
    explicit Posicion(GameWindow *gw, QWidget *parent = nullptr, int h = 100, int pos = 0, QString token = "", QWebSocket *ws = nullptr);
    void mostrarPosicion();
    int player_id;
    void setCard(Carta* carta);
    void removeCard();
    void setLock(bool l);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QWebSocket *ws;
    QString token;
    GameWindow *gw;
    int pos;
    int alturaCarta;
    int anchuraCarta;
    bool Lock;
    Carta* cartaActual;
    QPixmap fondoBase;

    void jugarCarta();
};

#endif // POSICION_H
