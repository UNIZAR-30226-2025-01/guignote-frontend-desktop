#include "clickableImage.h"
#include <QDebug>

ClickableImage::ClickableImage(QWidget *parent) : QLabel(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setScaledContents(true); // Permite que la imagen se ajuste al tamaño del QLabel
}

// Método para establecer la imagen y su tamaño
void ClickableImage::setImage(const QString &imagePath, int width, int height) {
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {  // Verifica si la imagen no se ha cargado correctamente
        qDebug() << "Error: No se pudo cargar la imagen desde " << imagePath;
        return;
    }

    setPixmap(pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setFixedSize(width, height);
}

// Detectar el clic y emitir la señal
void ClickableImage::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}
