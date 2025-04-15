#include "posicion.h"
#include <QPainter>
#include <QApplication>
#include "gamewindow.h"

Posicion::Posicion(GameWindow *gw, QWidget *parent, int h, int pos)
    : QLabel(parent), cartaActual(nullptr)
{
    this->alturaCarta = h;
    this->anchuraCarta = static_cast<int>(h * 0.7);  // Aproximadamente proporción carta
    this->gw = gw;
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

    if(pos != 0){
        Lock = true;
    } else {
        Lock = false;
    }

    this->mostrarPosicion();
}

void Posicion::mostrarPosicion()
{
    QWidget *contenedor = parentWidget();
    if (!contenedor) return;

    int espacioDesdeBorde = 200;
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
    event->acceptProposedAction();  // ✅ Aceptamos cualquier tipo de arrastre por ahora
    qDebug() << "dragEnterEvent";
}

void Posicion::dropEvent(QDropEvent *event)
{
    qDebug() << "dropEvent ACTIVADO";
    QString id = event->mimeData()->text();
    Carta* carta = gw->getCartaPorId(id);

    if (carta && !carta->isHidden() && !Lock) {
        carta->eliminarDeMano();
        carta->setParent(this);
        carta->move(0, 0);
        carta->show();
        carta->raise();

        cartaActual = carta;

        qDebug() << "Carta jugada" << carta->num << carta->suit;
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

