/**
 * @file posicion.cpp
 * @brief Implementación de la clase Posicion.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Este archivo define la clase Posicion, responsable de representar
 * cada zona de juego donde puede caer una carta. Gestiona su posición,
 * rotación, drag&drop y notifica al servidor cuando se juega una carta.
 */


#include "posicion.h"
#include <QPainter>
#include <QApplication>
#include <QtWebSockets>
#include "gamewindow.h"

/**
 * @brief Constructor de Posicion.
 * @param gw Puntero a la ventana de juego (GameWindow).
 * @param parent Widget padre en el que se dibuja la posición.
 * @param h Altura de cada carta en píxeles.
 * @param pos Índice de la posición (0=abajo,1=arriba,2=derecha,3=izquierda).
 * @param token Cadena de autenticación para el servidor.
 * @param ws WebSocket usado para comunicar jugadas.
 *
 * Crea un QLabel con fondo semitransparente, fija su tamaño,
 * habilita drag&drop y lo sitúa según el índice pos.
 */
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

/**
 * @brief Reposiciona y rota el widget según su índice.
 *
 * Calcula las coordenadas dentro de su contenedor y, si
 * corresponde, rota el fondo en 90°, 180° o 270°.
 */
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

/**
 * @brief Evento de entrada de arrastre.
 * @param event Información del evento de drag.
 *
 * Acepta cualquier arrastre para poder soltar cartas aquí.
 */
void Posicion::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();  // ✅ Aceptamos cualquier tipo de arrastre

}

/**
 * @brief Evento de soltado de una carta.
 * @param event Información del evento de drop.
 *
 * Si no está bloqueada y no hay carta ya, extrae el id del MIME,
 * obtiene la Carta desde GameWindow, la mueve, y llama a jugarCarta().
 */
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

/**
 * @brief Envía al servidor la jugada de la carta actual.
 *
 * Construye un JSON con palo y valor, lo serializa y lo emite
 * por el WebSocket si está conectado.
 */
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

/**
 * @brief Coloca una carta en esta posición.
 * @param carta Puntero a la carta que se va a fijar aquí.
 *
 * Ajusta padre, posición y visibilidad, y guarda la cartaActual.
 */
void Posicion::setCard(Carta *carta) {
    carta->setParent(this);
    int x = (width() - carta->width()) / 2;
    int y = (height() - carta->height()) / 2;
    carta->move(x, y);
    carta->show();
    carta->raise();

    cartaActual = carta;
}

/**
 * @brief Elimina la carta actual de la posición.
 *
 * Oculta y destruye el objeto Carta y deja cartaActual a nullptr.
 */
void Posicion::removeCard(){
    if(cartaActual){
        cartaActual->hide();
        cartaActual->deleteLater();
        cartaActual = nullptr;
    }
}

/**
 * @brief Bloquea o desbloquea esta posición.
 * @param l true para bloquear (no aceptar drops), false para desbloquear.
 */
void Posicion::setLock(bool l){
    Lock = l;
}
