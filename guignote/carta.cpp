#include "carta.h"
#include <QPainter>
#include <QMouseEvent>

Carta::Carta(QWidget *parent, QString num, QString suit, int h, int skin)
    : QLabel(parent), arrastrando(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    setMouseTracking(true);

    QPixmap img = selectPixmap(num, suit, skin);
    setImagen(img, h);
}

QPixmap Carta::selectPixmap(QString num, QString suit, int skin){
    QString ruta = ":/decks/";
    //Seleccionamos la skin
    switch (skin) {
    case 0:
        ruta += "base/";
        break;
    case 1:
        ruta += "poker/";
        break;
    }

    //Seleccionamos imagen
    if (num == "0"){
        ruta+="Back";
    } else {
        ruta+=num;
        ruta+=suit;
    }

    //Extension de archivo
    ruta+=".png";

    QPixmap pixmap(ruta);
    return pixmap;
}

void Carta::setImagen(const QPixmap &pixmap, int h)
{
    imagen = pixmap;
    update();  // Redibuja el widget

    int originalWidth = pixmap.width();
    int originalHeight = pixmap.height();
    double aspectRatio = static_cast<double>(originalWidth) / originalHeight;

    int newWidth = static_cast<int>(h * aspectRatio);

    // Ajustar el tamaño del ImageButton
    setFixedSize(newWidth, h);
    setPixmap(pixmap.scaled(newWidth, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Carta::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        arrastrando = true;
        offsetArrastre = event->pos();
    }
}

void Carta::mouseMoveEvent(QMouseEvent *event)
{
    if (arrastrando && (event->buttons() & Qt::LeftButton)) {
        move(mapToParent(event->pos() - offsetArrastre));
    }
}
