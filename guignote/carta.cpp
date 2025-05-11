#include "carta.h"
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsOpacityEffect>

int Carta::skin = 0;

Carta::Carta(QWidget* parent)
    : Carta("Back", "", parent) {}

Carta::Carta(const QString& palo, const QString& valor, QWidget *parent)
    : palo(palo), valor(valor), orientacion(Orientacion::DOWN), QLabel(parent) {
        cargarImagen();
}

void Carta::setPosicion(int x, int y) {
    this->posX = x;
    this->posY = y;
    this->move(posX, posY);
}

void Carta::setPaloValor(const QString& palo, const QString& valor) {
    this->palo = palo;
    this->valor = valor;
    cargarImagen();
    setOrientacion(this->orientacion);
    this->show();
}

QString Carta::getPalo() const {
    return palo;
}

QString Carta::getValor() const {
    return valor;
}

void Carta::setOrientacion(Orientacion orientacion) {
    this->orientacion = orientacion;
    if(!img[0].isNull()) {
        this->setPixmap(img[orientacion % 2]);
        this->resize(img[orientacion % 2].size());
    }
}

void Carta::cargarImagen() {
    QString _skin = "base";
    if(Carta::skin == 1) _skin = "poker";

    QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();

    img[0] = QPixmap(":/decks/" + _skin + "/" + valor + palo + ".png")
    .scaled(screenSize.width() * .05f, screenSize.height() * .1f, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QTransform transform = QTransform();
    transform.rotate(90);
    img[1] = img[0].transformed(transform);
    this->setPixmap(img[orientacion % 2]);
    this->resize(img[orientacion % 2].size());
}

void Carta::enterEvent(QEnterEvent* event) {
    if (!interactuable) return;
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.33);
    this->setGraphicsEffect(effect);
    QLabel::enterEvent(event);
}

void Carta::leaveEvent(QEvent* event) {
    if (!interactuable) return;
    this->setGraphicsEffect(nullptr);
    QLabel::leaveEvent(event);
}

void Carta::mouseDoubleClickEvent(QMouseEvent* event) {
    if(interactuable) {
        this->setGraphicsEffect(nullptr);
        emit cartaDobleClick(this);
    }
    QLabel::mouseDoubleClickEvent(event);
}
