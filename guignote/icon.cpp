/**
 * @file icon.cpp
 * @brief Implementación de la clase Icon.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de una etiqueta gráfica interactiva (Icon),
 * que gestiona la carga de imágenes, escalado y detección de eventos
 * de clic y hover.
 */


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
}

/**
 * @brief Asigna un QPixmap ya cargado y lo escala al tamaño dado.
 * @param pixmap QPixmap de origen.
 * @param width Ancho deseado del icono.
 * @param height Altura deseada del icono.
 *
 * Guarda internamente el pixmap original y ajusta el QLabel
 * para mantener el aspecto con suavizado.
 */

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

/**
 * @brief Agranda el icono cuando el cursor entra en su área.
 * @param event Información del evento de entrada.
 *
 * Si el hover está habilitado, escala el icono un 10% respecto
 * a su tamaño base y ajusta el QLabel.
 */

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

/**
 * @brief Restaura el tamaño original cuando el cursor sale.
 * @param event Información del evento de salida.
 *
 * Si el hover está habilitado, vuelve a escalar al tamaño base
 * y ajusta el QLabel.
 */

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
