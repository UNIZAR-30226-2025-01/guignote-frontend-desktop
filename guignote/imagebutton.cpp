#include "imagebutton.h"
#include <QVBoxLayout>
#include <QBitmap>
#include <QPainter>

ImageButton::ImageButton(const QString &imagePath, const QString &text, QWidget *parent)
    : QLabel(parent), normalPixmap(imagePath) {
    setAttribute(Qt::WA_TranslucentBackground);

    // Asegurar que la imagen mantiene su transparencia
    QImage img = normalPixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    normalPixmap = QPixmap::fromImage(img);
    darkenedPixmap = generateDarkenedPixmap(normalPixmap);
    setPixmap(normalPixmap);
    setFixedSize(normalPixmap.size());

    textLabel = new QLabel(text, this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold; text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.7); background: transparent;");
    textLabel->setGeometry(0, 0, width(), height());
    textLabel->hide();  // Ocultar texto al inicio
}

QPixmap ImageButton::generateDarkenedPixmap(const QPixmap &source) {
    QImage img = source.toImage().convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor color = QColor::fromRgba(line[x]);

            // Si el píxel no es completamente transparente
            if (color.alpha() > 0) {
                int r = qMax(color.red() * 0.5, 0.0);   // Reduce brillo al 50%
                int g = qMax(color.green() * 0.5, 0.0);
                int b = qMax(color.blue() * 0.5, 0.0);
                line[x] = QColor(r, g, b, color.alpha()).rgba(); // Mantiene la transparencia
            }
        }
    }

    return QPixmap::fromImage(img);
}



void ImageButton::enterEvent(QEnterEvent *event) {
    Q_UNUSED(event);
    setPixmap(darkenedPixmap);
    textLabel->show();
}

void ImageButton::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    setPixmap(normalPixmap);
    textLabel->hide();
}

void ImageButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}
