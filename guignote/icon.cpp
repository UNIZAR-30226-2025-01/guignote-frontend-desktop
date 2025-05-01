#include "icon.h"
#include <QDebug>

/**
 * @brief Constructor de la clase Icon.
 * @param parent Widget padre, por defecto es nullptr.
 *
 * Inicializa el QLabel con el widget padre y configura atributos específicos,
 * como el fondo translúcido y el escalado automático del contenido.
 */
Icon::Icon(QWidget *parent) : QLabel(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    hoverEnabled = true;
    setScaledContents(true); // Permite que la imagen se ajuste al tamaño del QLabel
}

/**
 * @brief Establece la imagen y su tamaño.
 * @param imagePath Ruta de la imagen a cargar.
 * @param width Ancho deseado para la imagen.
 * @param height Altura deseada para la imagen.
 *
 * Carga la imagen desde la ruta especificada y verifica si se ha cargado correctamente.
 * Si la imagen es válida, se escala manteniendo el aspecto y se fija el tamaño del QLabel.
 * En caso contrario, se imprime un mensaje de error en la consola de depuración.
 */
void Icon::setImage(const QString &imagePath, int width, int height) {
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {  // Verifica si la imagen no se ha cargado correctamente
        qDebug() << "Error: No se pudo cargar la imagen desde " << imagePath;
        return;
    }

    // Guardamos la información para reescalados posteriores
    originalPixmap = pixmap;
    baseWidth = width;
    baseHeight = height;

    setPixmap(pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setFixedSize(width, height);

    setAttribute(Qt::WA_TranslucentBackground);
    setScaledContents(true);

    // Creamos la etiqueta del badge
    badgeLabel = new QLabel(this);
    badgeLabel->setStyleSheet(R"(
        background-color: red;
        color: white;
        font-size: 10px;
        border-radius: 8px;
    )");
    badgeLabel->setAlignment(Qt::AlignCenter);
    badgeLabel->setFixedSize(16,16);
    badgeLabel->hide();

    // Preparamos la animación de shake
    shakeAnim = new QPropertyAnimation(this, "pos", this);
    shakeAnim->setDuration(300);
    shakeAnim->setKeyValueAt(0.0,   pos());
    shakeAnim->setKeyValueAt(0.10,  pos() + QPoint(-5, 0));
    shakeAnim->setKeyValueAt(0.20,  pos() + QPoint( 5, 0));
    shakeAnim->setKeyValueAt(0.30,  pos() + QPoint(-5, 0));
    shakeAnim->setKeyValueAt(0.40,  pos() + QPoint( 5, 0));
    shakeAnim->setKeyValueAt(0.50,  pos());
    // (el resto vuelve al 1.0 en pos())
    shakeAnim->setKeyValueAt(1.0,   pos());
}

void Icon::setPixmapImg(const QPixmap &pixmap, int width, int height) {
    originalPixmap = pixmap;
    baseWidth = width;
    baseHeight = height;
    QLabel::setPixmap(pixmap.scaled(width, height,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    setFixedSize(width, height);
}

/**
 * @brief Maneja el evento de clic del mouse.
 * @param event Evento de tipo QMouseEvent que contiene la información del clic.
 *
 * Detecta si se ha hecho clic con el botón izquierdo del mouse y, de ser así,
 * emite la señal clicked() para notificar a otros componentes.
 */
void Icon::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}

// 3) Amplía el icono al entrar el ratón
void Icon::enterEvent(QEnterEvent *event) {
    if (!hoverEnabled) return;
    // Aumentamos un 10% el tamaño, por ejemplo
    int newW = static_cast<int>(baseWidth * 1.1);
    int newH = static_cast<int>(baseHeight * 1.1);

    setPixmap(originalPixmap.scaled(newW, newH,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    setFixedSize(newW, newH);

    QLabel::enterEvent(event); // Llamada al padre
}

// 4) Vuelve al tamaño original cuando el ratón sale
void Icon::leaveEvent(QEvent *event) {
    if (!hoverEnabled) return;
    setPixmap(originalPixmap.scaled(baseWidth, baseHeight,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    setFixedSize(baseWidth, baseHeight);

    QLabel::leaveEvent(event);
}

/**
 * @brief Bloquea la detección de hover para este objeto Icon.
 * @param enabled Si es false, el hover estará deshabilitado.
 */
void Icon::setHoverEnabled(bool enabled) {
    hoverEnabled = enabled;
}


void Icon::resizeEvent(QResizeEvent *event) {
    QLabel::resizeEvent(event);
    // Recolocamos el badge en la esquina superior derecha
    int bx = width()  - badgeLabel->width()/2;
    int by = - badgeLabel->height()/2;
    badgeLabel->move(bx, by);
}

void Icon::setBadgeCount(int count) {
    badgeCount = count;
    if (count > 0) {
        badgeLabel->setText(QString::number(count));
        badgeLabel->show();
        shake();
    } else {
        badgeLabel->hide();
    }
}

void Icon::shake() {
    if (shakeAnim->state() == QAbstractAnimation::Running)
        shakeAnim->stop();
    // Actualizamos los valores por si el icono se ha movido
    QPoint p0 = pos();
    shakeAnim->setKeyValueAt(0.0,  p0);
    shakeAnim->setKeyValueAt(0.10, p0 + QPoint(-5,0));
    shakeAnim->setKeyValueAt(0.20, p0 + QPoint( 5,0));
    shakeAnim->setKeyValueAt(0.30, p0 + QPoint(-5,0));
    shakeAnim->setKeyValueAt(0.40, p0 + QPoint( 5,0));
    shakeAnim->setKeyValueAt(0.50, p0);
    shakeAnim->setKeyValueAt(1.0,  p0);
    shakeAnim->start();
}
