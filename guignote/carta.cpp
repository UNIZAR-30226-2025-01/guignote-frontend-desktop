/**
 * @file carta.cpp
 * @brief Implementación de la clase Carta.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene las definiciones de los métodos de la clase Carta,
 * que representa una carta en la interfaz de juego, con soporte
 * de drag & drop, giro (anverso/reverso) y asociación a Mano.
 */

#include "carta.h"
#include "mano.h"
#include "gamewindow.h"

#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDebug>

/**
 * @brief Constructor de la carta.
 * @param gw Puntero a la ventana de juego (para registrar la carta).
 * @param parent Widget padre.
 * @param num Número o figura de la carta.
 * @param suit Palo de la carta.
 * @param h Altura en píxeles a la que escalar la carta.
 * @param skin Estilo gráfico (skin) de la carta.
 * @param faceUp True para mostrar el anverso al crear.
 */
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

    // Selección de pixmaps de anverso y reverso
    QPixmap front = selectPixmap(skin);
    backPixmap    = selectPixmap(0);               // skin=0 corresponde al reverso
    setImagen(front, h);

    // Guardamos pixmaps escalados para uso posterior
    frontPixmap = this->pixmap();
    pixmapOrig  = frontPixmap;
    backPixmap  = backPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Identificador global y registro en GameWindow
    idGlobal = num + suit;
    if (gw)
        gw->addCartaPorId(this);
}

/**
 * @brief Establece el estado de bloqueo de la carta.
 * @param lock True para bloquear interacción, false para desbloquear.
 */
void Carta::setLock(bool lock) {
    locked = lock;
}

/**
 * @brief Obtiene la imagen actual de la carta.
 * @return QPixmap con la imagen mostrada.
 */
QPixmap Carta::getImagen() const {
    return imagen;
}

/**
 * @brief Selecciona el pixmap según el skin especificado.
 * @param skin Estilo gráfico.
 * @return QPixmap cargado desde recursos.
 */
QPixmap Carta::selectPixmap(int skin) const {
    QString ruta = ":/decks/";
    ruta += (skin == 1 ? "poker/" : "base/");

    if (num == "0")
        ruta += "Back";
    else
        ruta += num + suit;

    ruta += ".png";
    qDebug() << "🔍 Intentando cargar:" << ruta;
    QPixmap pixmap(ruta);
    if (pixmap.isNull())
        qWarning() << "Carta::selectPixmap: no existe" << ruta;
    return pixmap;
}

/**
 * @brief Establece la imagen mostrada y ajusta el tamaño del widget.
 * @param pixmap Imagen a mostrar.
 * @param h Altura deseada en píxeles.
 */
void Carta::setImagen(const QPixmap &pixmap, int h) {
    imagen = pixmap;
    update();

    double aspect = double(pixmap.width()) / pixmap.height();
    int newW = int(h * aspect);

    setFixedSize(newW, h);
    setPixmap(pixmap.scaled(newW, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/**
 * @brief Muestra el anverso de la carta.
 */
void Carta::reveal() {
    isFaceUp = true;
    setPixmap(frontPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/**
 * @brief Muestra el reverso de la carta.
 */
void Carta::hideFace() {
    isFaceUp = false;
    setPixmap(backPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/**
 * @brief Gestiona el evento de pulsación del ratón.
 * @param event Evento de ratón.
 */
void Carta::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !locked) {
        arrastrando = true;
        offsetArrastre = event->pos();
    }
    QLabel::mousePressEvent(event);
}

/**
 * @brief Gestiona el evento de movimiento del ratón para drag & drop.
 * @param event Evento de ratón.
 */
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

/**
 * @brief Gestiona el evento de liberación del ratón.
 * @param event Evento de ratón.
 */
void Carta::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !locked) {
        arrastrando = false;
        if (mano && ID != -1)
            mano->mostrarMano();
    }
    QLabel::mouseReleaseEvent(event);
}

/**
 * @brief Añade la carta a una mano.
 * @param m Puntero a la Mano.
 * @param id Índice en la mano.
 */
void Carta::añadirAMano(Mano *m, int id) {
    mano = m;
    ID   = id;
}

/**
 * @brief Elimina la carta de la mano asociada.
 */
void Carta::eliminarDeMano() {
    if (mano && ID >= 0) {
        mano->eliminarCarta(ID);
        ID   = -1;
        mano = nullptr;
    }
}

/**
 * @brief Obtiene el pixmap original no escalado.
 * @return QPixmap original.
 */
QPixmap Carta::getOriginalPixmap() const {
    return pixmapOrig;
}
