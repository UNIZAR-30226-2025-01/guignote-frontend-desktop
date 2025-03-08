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

    setPixmap(pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
