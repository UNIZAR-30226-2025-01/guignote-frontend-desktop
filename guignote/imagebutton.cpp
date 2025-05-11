/**
 * @file imagebutton.cpp
 * @brief Implementación de la clase ImageButton.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de un botón basado en imagen con efecto hover,
 * que oscurece la imagen y muestra texto al pasar el cursor, así como
 * redimensionamiento manteniendo la relación de aspecto.
 */


#include "imagebutton.h"
#include <QVBoxLayout>
#include <QBitmap>
#include <QFontDatabase>
#include <QPainter>

/**
 * @brief Constructor de ImageButton.
 * @param imagePath Ruta de la imagen a cargar.
 * @param text Texto que se mostrará sobre la imagen.
 * @param parent Widget padre, por defecto es nullptr.
 *
 * Inicializa el botón de imagen cargando la imagen normal y generando una versión
 * oscurecida para los efectos interactivos. Además, configura una fuente personalizada
 * para el texto, ajusta la transparencia y define el tamaño del widget.
 */
ImageButton::ImageButton(const QString &imagePath, const QString &text, QWidget *parent)
    : QLabel(parent), normalPixmap(imagePath) {
    setAttribute(Qt::WA_TranslucentBackground);

    // Cargar la fuente personalizada.
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QFont titleFont;
    if (fontId != -1) {
        titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    } else {
        qWarning() << "No se pudo cargar la fuente personalizada.";
        titleFont = QFont("Arial", 32, QFont::Bold);
    }

    // Asegurar que la imagen mantiene su transparencia.
    QImage img = normalPixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    normalPixmap = QPixmap::fromImage(img);
    // Generar la imagen oscurecida para el efecto al pasar el cursor.
    darkenedPixmap = generateDarkenedPixmap(normalPixmap);
    setPixmap(normalPixmap);
    setFixedSize(normalPixmap.size());

    // Crear y configurar la etiqueta de texto.
    textLabel = new QLabel(text, this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold; background: transparent;");
    textLabel->setGeometry(0, 0, width(), height());
    textLabel->setFont(titleFont);
    textLabel->hide();  // Ocultar texto al inicio.
}

/**
 * @brief Genera una versión oscurecida del QPixmap proporcionado.
 * @param source QPixmap original.
 * @return QPixmap Imagen oscurecida.
 *
 * Este método recorre cada píxel de la imagen fuente, reduciendo su brillo al 50%
 * y manteniendo la transparencia para lograr un efecto de oscurecimiento.
 */
QPixmap ImageButton::generateDarkenedPixmap(const QPixmap &source) {
    QImage img = source.toImage().convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor color = QColor::fromRgba(line[x]);

            // Si el píxel no es completamente transparente.
            if (color.alpha() > 0) {
                int r = qMax(color.red() * 0.5, 0.0);   // Reducir brillo al 50%.
                int g = qMax(color.green() * 0.5, 0.0);
                int b = qMax(color.blue() * 0.5, 0.0);
                line[x] = QColor(r, g, b, color.alpha()).rgba(); // Mantener la transparencia.
            }
        }
    }

    return QPixmap::fromImage(img);
}

/**
 * @brief Evento al entrar el cursor en el área del widget.
 * @param event Evento de entrada del ratón (QEnterEvent).
 *
 * Al detectar el evento, se cambia la imagen a la versión oscurecida y se muestra el texto.
 */
void ImageButton::enterEvent(QEnterEvent *event) {
    Q_UNUSED(event);
    setPixmap(darkenedPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    textLabel->show();
}

/**
 * @brief Evento al salir el cursor del área del widget.
 * @param event Evento de salida del ratón (QEvent).
 *
 * Restaura la imagen a su estado normal y oculta el texto cuando el cursor abandona el widget.
 */
void ImageButton::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    setPixmap(normalPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    textLabel->hide();
}

/**
 * @brief Evento de presión del botón del ratón.
 * @param event Evento de tipo QMouseEvent.
 *
 * Detecta un clic con el botón izquierdo del ratón y emite la señal clicked().
 */
void ImageButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}

/**
 * @brief Ajusta dinámicamente el tamaño del botón manteniendo la relación de aspecto.
 * @param h Altura de referencia (por ejemplo, altura de la ventana).
 * @return QSize Nuevo tamaño calculado para el botón.
 *
 * Escala el QPixmap original de acuerdo a la nueva altura (dividida entre 3),
 * aplica límites mínimos y máximos, y reajusta la posición del texto centrado.
 */

QSize ImageButton::updatesize(int h) {
    if (normalPixmap.isNull()) return QSize(0, 0);

    int originalWidth = normalPixmap.width();
    int originalHeight = normalPixmap.height();
    double aspectRatio = static_cast<double>(originalWidth) / originalHeight;

    int newHeight = h / 3;  // Escalar basado en la altura de la ventana.
    int newWidth = static_cast<int>(newHeight * aspectRatio);

    // Definir valores mínimos y máximos.
    const int minHeight = 200;
    const int maxHeight = 800;

    // Ajustar a los límites establecidos.
    if (newHeight < minHeight) {
        newHeight = minHeight;
        newWidth = static_cast<int>(newHeight * aspectRatio);
    }
    if (newHeight > maxHeight) {
        newHeight = maxHeight;
        newWidth = static_cast<int>(newHeight * aspectRatio);
    }

    // Ajustar el tamaño del ImageButton
    setFixedSize(newWidth, newHeight);
    setPixmap(normalPixmap.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // ✅ Ajustar la posición y tamaño de textLabel para que siga centrado
    textLabel->setGeometry(0, 0, newWidth, newHeight);

    return QSize(newWidth, newHeight);
}

