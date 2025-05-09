// carta.cpp
#include "carta.h"
#include "mano.h"
#include "gamewindow.h"

#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDebug>

Carta::Carta(GameWindow *gw,
             QWidget *parent,
             const QString &num,
             const QString &suit,
             int h,
             int skin,
             bool faceUp)
    : QLabel(parent),
    num(num),
    suit(suit),
    isFaceUp(faceUp),
    locked(true),
    arrastrando(false),
    ID(-1),
    mano(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    setMouseTracking(true);

    // Selección de imagenes
    QPixmap front = selectPixmap(skin);
    backPixmap    = selectPixmap(0);               // asumimos que skin=0 es reverso
    setImagen(front, h);

    // Guardamos pixmaps escalados
    frontPixmap = this->pixmap();                  // cara visible
    pixmapOrig  = frontPixmap;
    backPixmap  = backPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    idGlobal = num + suit;
    if (gw) gw->addCartaPorId(this);
}

void Carta::setLock(bool lock) {
    locked = lock;
}

QPixmap Carta::getImagen() const {
    return imagen;
}

QPixmap Carta::selectPixmap(int skin) const {
    QString ruta = ":/decks/";
    ruta += (skin == 1 ? "poker/" : "base/");

    if (num == "0")
        ruta += "Back";
    else
        ruta += num + suit;

    ruta += ".png";
    qDebug() << "🔍 Intentando cargar:" << ruta; // 👈 AÑADE ESTO
    QPixmap pixmap(ruta);
    if (pixmap.isNull())
        qWarning() << "Carta::selectPixmap: no existe" << ruta;
    return pixmap;
}

void Carta::setImagen(const QPixmap &pixmap, int h) {
    imagen = pixmap;
    update();

    double aspect = double(pixmap.width())/pixmap.height();
    int newW = int(h * aspect);

    setFixedSize(newW, h);
    setPixmap(pixmap.scaled(newW, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Carta::reveal() {
    isFaceUp = true;
    setPixmap(frontPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Carta::hideFace() {
    isFaceUp = false;
    setPixmap(backPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Carta::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !locked) {
        arrastrando = true;
        offsetArrastre = event->pos();
    }
    QLabel::mousePressEvent(event);
}

void Carta::mouseMoveEvent(QMouseEvent *event) {
    if (!locked && (event->buttons() & Qt::LeftButton)) {
        if ((event->pos() - offsetArrastre).manhattanLength() < QApplication::startDragDistance())
            return;

        QMimeData *mime = new QMimeData;
        mime->setText(idGlobal);

        QDrag drag(this);
        drag.setMimeData(mime);
        drag.setPixmap(pixmap());
        drag.setHotSpot(event->pos());

        hide();
        if (drag.exec(Qt::MoveAction) == Qt::IgnoreAction)
            show();
    }
    QLabel::mouseMoveEvent(event);
}

void Carta::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !locked) {
        arrastrando = false;
        if (mano && ID != -1)
            mano->mostrarMano();
    }
    QLabel::mouseReleaseEvent(event);
}

void Carta::añadirAMano(Mano *m, int id) {
    mano = m;
    ID   = id;
}

void Carta::eliminarDeMano() {
    if (mano && ID >= 0) {
        mano->eliminarCarta(ID);
        ID   = -1;
        mano = nullptr;
    }
}

QPixmap Carta::getOriginalPixmap() const {
    return pixmapOrig;
}
