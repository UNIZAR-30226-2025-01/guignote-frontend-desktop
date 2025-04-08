#include "carta.h"
#include "mano.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

Carta::Carta(QWidget *parent, QString num, QString suit, int h, int skin)
    : QLabel(parent), arrastrando(false)
{
    this->num = num;
    this->suit = suit;
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    setMouseTracking(true);

    QPixmap img = selectPixmap(skin);
    setImagen(img, h);

    ID = -1;
    this->locked = true;

    pixmapOrig = this->pixmap(); // Una sola vez al inicio
}

void Carta::setLock(bool lock){
    locked=lock;
}

QPixmap Carta::getImagen() const {
    return imagen;
}

QPixmap Carta::selectPixmap(int skin){
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
        ruta+=this->num;
        ruta+=this->suit;
    }

    //Extension de archivo
    ruta+=".png";

    qDebug() << "ruta: " << ruta;

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
    if (event->button() == Qt::LeftButton && !locked) {
        arrastrando = true;
        offsetArrastre = event->pos();
    }
}



void Carta::mouseMoveEvent(QMouseEvent *event)
{
    if (arrastrando && (event->buttons() & Qt::LeftButton) && !locked) {
        move(mapToParent(event->pos() - offsetArrastre));
    }
}

void Carta::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !locked) {
        arrastrando = false;

        if (mano && ID != -1) {
            // suelta en algún lugar válido o vuelve a la mano
            mano->mostrarMano();  // vuelve a colocar la mano
            qDebug() << "Carta reordenada";
        } else {
            qDebug() << "Carta soltada sin estar en mano";
        }
    }
}


//Gestión de mano
void Carta::añadirAMano(Mano *mano, int id){
    ID = id;
    this->mano = mano;
}

void Carta::eliminarDeMano(){
    mano->eliminarCarta(ID);
    ID = -1;
}
