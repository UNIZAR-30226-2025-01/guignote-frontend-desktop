/**
 * @file deck.cpp
 * @brief Implementación de la clase Deck.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de la clase Deck, que representa un mazo de cartas
 * en la interfaz de usuario, incluyendo la visualización del fondo del mazo,
 * la carta de triunfo girada, y un contador de cartas restantes.
 */

#include "deck.h"
#include <QResizeEvent>

/**
 * @brief Constructor de la clase Deck.
 * @param triunfo Puntero a la carta de triunfo.
 * @param skin Estilo gráfico del mazo.
 * @param cardSize Altura deseada para las cartas.
 * @param parent Widget padre.
 * @param token Token identificador del mazo.
 */
Deck::Deck(Carta* triunfo, int skin, int cardSize, QWidget *parent, QString token)
    : QWidget(parent), triunfo(triunfo), skin(skin), num(40)
{
    this->token = token;

    fondo = new QLabel(this);
    QString rutaFondo = (skin == 1) ? ":/decks/poker/Back.png" : ":/decks/base/Back.png";
    QPixmap pix(rutaFondo);
    setImagenFondo(pix, cardSize);

    contador = new QLabel(QString::number(num), this);
    contador->setStyleSheet(R"(
        color: white;
        font: bold 20px;
        background-color: rgba(0, 0, 0, 180);
        border-radius: 10px;
        padding: 4px;
    )");
    contador->setAlignment(Qt::AlignCenter);
    contador->adjustSize();

    int anchoDeck = fondo->width();
    int altoDeck = fondo->height();

    if (triunfo) {
        QPixmap original = triunfo->getOriginalPixmap();
        QTransform rotacion;
        rotacion.rotate(90);
        QPixmap rotada = original.transformed(rotacion, Qt::SmoothTransformation);

        triunfo->setPixmap(rotada);
        triunfo->setFixedSize(rotada.size());
        triunfo->setParent(this);
        triunfo->lower();
        triunfo->show();

        anchoDeck += triunfo->width();
        altoDeck = std::max(altoDeck, triunfo->height());
    }

    int altoTotal = altoDeck + contador->height();
    setFixedSize(anchoDeck, altoTotal);

    actualizarVisual();
}

/**
 * @brief Establece la imagen del fondo del mazo.
 * @param pixmap Imagen a utilizar como fondo.
 * @param alturaDeseada Altura en píxeles para escalar.
 */
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

/**
 * @brief Recalcula posiciones de los elementos visuales del mazo.
 */
void Deck::actualizarVisual()
{
    if (!parentWidget()) return;

    int fondoHeight = fondo->height();
    int triunfoHeight = triunfo ? triunfo->height() : 0;
    int altoVisible = std::max(fondoHeight, triunfoHeight);
    int altoExtra = contador->height();

    int xCentro = (parentWidget()->width() - width()) / 2;
    int yCentro = (parentWidget()->height() - altoVisible) / 2;

    move(xCentro, yCentro - altoExtra);

    int fondoY = altoExtra;
    fondo->move(0, fondoY);

    int contadorX = (fondo->width() - contador->width()) / 2;
    int contadorY = fondoY - contador->height();
    contador->move(contadorX, contadorY);
    contador->raise();

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

/**
 * @brief Gestiona el evento de redimensionamiento del widget.
 * @param event Evento de cambio de tamaño.
 */
void Deck::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    actualizarVisual();
}

/**
 * @brief Decrementa el número de cartas y actualiza el contador.
 */
void Deck::cartaRobada(){
    num--;
    contador->setText(QString::number(num));
    qDebug() << "Se hizo clic en el mazo. Cartas restantes:" << num;
}

/**
 * @brief Establece una nueva carta de triunfo.
 * @param nuevaTriunfo Puntero a la nueva carta de triunfo.
 */
void Deck::setTriunfo(Carta* nuevaTriunfo)
{
    if (triunfo) {
        triunfo->hide();
        triunfo->setParent(nullptr);
        triunfo = nullptr;
    }

    if (nuevaTriunfo) {
        triunfo = nuevaTriunfo;

        QPixmap original = triunfo->getOriginalPixmap();
        QTransform rotacion;
        rotacion.rotate(90);
        QPixmap rotada = original.transformed(rotacion, Qt::SmoothTransformation);

        triunfo->setPixmap(rotada);
        triunfo->setFixedSize(rotada.size());
        triunfo->setParent(this);
        triunfo->lower();
        triunfo->show();

        int anchoDeck = fondo->width() + triunfo->width();
        int altoDeck = std::max(fondo->height(), triunfo->height());
        int altoTotal = altoDeck + contador->height();
        setFixedSize(anchoDeck, altoTotal);
    }
    actualizarVisual();
}

/**
 * @brief Establece el número de cartas restantes y actualiza la vista.
 * @param n Nuevo número de cartas.
 */
void Deck::setNum(int n){
    num = n;
    contador->setText(QString::number(num));
    actualizarVisual();
}
