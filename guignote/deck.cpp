#include "deck.h"
#include <QResizeEvent>


Deck::Deck(Carta* triunfo, int skin, int cardSize, QWidget *parent)
    : QWidget(parent), triunfo(triunfo), skin(skin), num(40)
{
    // Fondo del mazo
    fondo = new QLabel(this);
    QString rutaFondo = (skin == 1) ? ":/decks/poker/Back.png" : ":/decks/base/Back.png";
    QPixmap pix(rutaFondo);
    setImagenFondo(pix, cardSize);

    // Contador encima del mazo
    contador = new QLabel(QString::number(num), this);
    contador->setStyleSheet(R"(
        color: white;
        font: bold 20px;
        background-color: rgba(0, 0, 0, 180);
        border-radius: 10px;
        padding: 4px;
    )");
    contador->setAlignment(Qt::AlignCenter);
    contador->adjustSize();  // ajusta el tamaño al contenido

    // Dimensiones base del Deck
    int anchoDeck = fondo->width();
    int altoDeck = fondo->height();

    // Carta de triunfo
    if (triunfo) {
        QPixmap original = triunfo->pixmapOrig;
        QTransform rotacion;
        rotacion.rotate(90);
        QPixmap rotada = original.transformed(rotacion, Qt::SmoothTransformation);

        triunfo->setPixmap(rotada);
        triunfo->setFixedSize(rotada.size());
        triunfo->setParent(this);
        triunfo->lower();  // detrás del fondo
        triunfo->show();

        // Aumentar ancho y alto si hace falta
        anchoDeck += triunfo->width();
        altoDeck = std::max(altoDeck, triunfo->height());
    }

    // Añadir altura del contador pero no considerarla para centrado
    int altoTotal = altoDeck + contador->height();
    setFixedSize(anchoDeck, altoTotal);

    actualizarVisual();
}


void Deck::setImagenFondo(const QPixmap &pixmap, int alturaDeseada)
{
    if (fondo) {
        int originalWidth = pixmap.width();
        int originalHeight = pixmap.height();
        double aspectRatio = static_cast<double>(originalWidth) / originalHeight;

        int newWidth = static_cast<int>(alturaDeseada * aspectRatio);

        fondo->setFixedSize(newWidth, alturaDeseada);
        fondo->setPixmap(pixmap.scaled(newWidth, alturaDeseada, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void Deck::actualizarVisual()
{
    if (!parentWidget()) return;

    int fondoHeight = fondo->height();
    int triunfoHeight = triunfo ? triunfo->height() : 0;
    int altoVisible = std::max(fondoHeight, triunfoHeight);
    int altoExtra = contador->height();

    // Centramos solo respecto a la parte visible (fondo + triunfo)
    int xCentro = (parentWidget()->width() - width()) / 2;
    int yCentro = (parentWidget()->height() - altoVisible) / 2;

    // Compensar el espacio extra por el contador
    move(xCentro, yCentro - altoExtra);

    // Posicionar el fondo del mazo
    int fondoY = altoExtra;  // dejar espacio arriba para el contador
    fondo->move(0, fondoY);

    // Contador centrado horizontalmente, justo encima del fondo
    int contadorX = (fondo->width() - contador->width()) / 2;
    int contadorY = fondoY - contador->height();
    contador->move(contadorX, contadorY);
    contador->raise();

    // Carta triunfo centrada verticalmente, parcialmente solapada
    if (triunfo) {
        int xTriunfo = fondo->width() - (triunfo->width() / 2);
        int yTriunfo = (height() - triunfo->height()) / 2;
        triunfo->move(xTriunfo, yTriunfo);
        triunfo->lower();
        fondo->raise();
    }
    if (num == 0) {
        hide();
    }
}


void Deck::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    actualizarVisual();
}

void Deck::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && num > 0) {
        num--;
        contador->setText(QString::number(num));
        qDebug() << "Se hizo clic en el mazo. Cartas restantes:" << num;
        actualizarVisual();
    }
}
