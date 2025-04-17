#include "posicion.h"
#include <QPainter>
#include <QApplication>
#include <QtWebSockets>
#include "gamewindow.h"

Posicion::Posicion(GameWindow *gw, QWidget *parent, int h, int pos, QString token, QWebSocket *ws)
    : QLabel(parent), cartaActual(nullptr)
{
    this->token = token;
    this->alturaCarta = h;
    this->anchuraCarta = static_cast<int>(h * 0.7);  // Aproximadamente proporción carta
    this->gw = gw;
    this->ws = ws;
    setFixedSize(anchuraCarta, alturaCarta);

    // Estilo visual: rectángulo gris semitransparente
    fondoBase = QPixmap(anchuraCarta, alturaCarta);
    fondoBase.fill(Qt::transparent);

    QPainter painter(&fondoBase);
    QColor color(0, 0, 0, 100);  // gris oscuro con opacidad
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, anchuraCarta, alturaCarta);

    // Mostramos inicialmente (sin rotación aún)
    setPixmap(fondoBase);

    setAcceptDrops(true);  // Muy importante para recibir drops

    this->pos = pos;

    Lock = true;

    this->mostrarPosicion();
}

void Posicion::mostrarPosicion()
{
    QWidget *contenedor = parentWidget();
    if (!contenedor) return;

    int espacioDesdeBorde = 225;
    int espacioDesdeBordeLados = 300;
    int espacioEntreZonas = 30;

    int anchoCarta = width();
    int altoCarta = height();

    switch (pos) {
    case 0: {  // Inferior
        int x = (contenedor->width() - anchoCarta) / 2;
        int y = contenedor->height() - altoCarta - espacioDesdeBorde;
        move(x, y);
        break;
    }
    case 1: {  // Superior
        int x = (contenedor->width() - anchoCarta) / 2;
        int y = espacioDesdeBorde;
        QTransform t;
        t.rotate(180);
        QPixmap rotado = fondoBase.transformed(t, Qt::SmoothTransformation);
        setPixmap(rotado);
        setFixedSize(rotado.size());
        move(x, y);
        break;
    }
    case 2: {  // Derecha
        int x = contenedor->width() - altoCarta - espacioDesdeBordeLados;
        int y = (contenedor->height() - anchoCarta) / 2;
        QTransform t;
        t.rotate(270);
        QPixmap rotado = fondoBase.transformed(t, Qt::SmoothTransformation);
        setPixmap(rotado);
        setFixedSize(rotado.size());
        move(x, y);
        break;
    }
    case 3: {  // Izquierda
        int x = espacioDesdeBordeLados;
        int y = (contenedor->height() - anchoCarta) / 2;
        QTransform t;
        t.rotate(90);
        QPixmap rotado = fondoBase.transformed(t, Qt::SmoothTransformation);
        setPixmap(rotado);
        setFixedSize(rotado.size());
        move(x, y);
        break;
    }
    default:
        break;
    }

    raise();
    show();
}

void Posicion::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();  // ✅ Aceptamos cualquier tipo de arrastre

}

void Posicion::dropEvent(QDropEvent *event)
{
    QString id = event->mimeData()->text();

    if (!Lock && gw && cartaActual == nullptr) {
        Carta* carta = gw->getCartaPorId(id);
        if (carta) {
            carta->eliminarDeMano();
            carta->setParent(this);
            int x = (width() - carta->width()) / 2;
            int y = (height() - carta->height()) / 2;
            carta->move(x, y);
            carta->show();
            carta->raise();

            cartaActual = carta;
            // Mandar mensaje por socket
            jugarCarta();

            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void Posicion::jugarCarta() {
    QJsonObject carta;
    carta["palo"] = cartaActual->suit;
    carta["valor"] = cartaActual->num.toInt();

    QJsonObject mensaje;
    mensaje["accion"] = "jugar_carta";
    mensaje["carta"] = carta;

    QJsonDocument doc(mensaje);
    QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    if (!ws) {
        qWarning() << "❌ WebSocket no está inicializado.";
        return;
    }

    if (ws->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "❌ WebSocket no está conectado. Estado:" << ws->state();
        return;
    }

    ws->sendTextMessage(jsonString);
}

void Posicion::setCard(Carta *carta) {
    carta->setParent(this);
    int x = (width() - carta->width()) / 2;
    int y = (height() - carta->height()) / 2;
    carta->move(x, y);
    carta->show();
    carta->raise();

    cartaActual = carta;
}

void Posicion::removeCard(){
    if(cartaActual){
        cartaActual->hide();
        cartaActual->deleteLater();
        cartaActual = nullptr;
    }
}

void Posicion::setLock(bool l){
    Lock = l;
}
