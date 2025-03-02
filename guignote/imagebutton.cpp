#include "imagebutton.h"
#include <QVBoxLayout>
#include <QBitmap>
#include <QFontDatabase>
#include <QPainter>

ImageButton::ImageButton(const QString &imagePath, const QString &text, QWidget *parent)
    : QLabel(parent), normalPixmap(imagePath) {
    setAttribute(Qt::WA_TranslucentBackground);

    // Cargar la fuente personalizada
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QFont titleFont;
    if (fontId != -1) {
        titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    } else {
        qWarning() << "No se pudo cargar la fuente personalizada.";
        titleFont = QFont("Arial", 32, QFont::Bold);
    }

    // Asegurar que la imagen mantiene su transparencia
    QImage img = normalPixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    normalPixmap = QPixmap::fromImage(img);
    darkenedPixmap = generateDarkenedPixmap(normalPixmap);
    setPixmap(normalPixmap);
    setFixedSize(normalPixmap.size());

    textLabel = new QLabel(text, this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold; background: transparent;");
    textLabel->setGeometry(0, 0, width(), height());
    textLabel->setFont(titleFont);
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
    setPixmap(darkenedPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    textLabel->show();
}

void ImageButton::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    setPixmap(normalPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    textLabel->hide();
}

void ImageButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}

QSize ImageButton::updatesize(int h) {
    if (normalPixmap.isNull()) return QSize(0, 0);

    int originalWidth = normalPixmap.width();
    int originalHeight = normalPixmap.height();
    double aspectRatio = static_cast<double>(originalWidth) / originalHeight;

    int newHeight = h / 3;  // Escalar basado en la altura de la ventana
    int newWidth = static_cast<int>(newHeight * aspectRatio);

    // Definir valores mínimos y máximos
    const int minHeight = 200;
    const int maxHeight = 800;

    // Ajustar a los límites establecidos
    if (newHeight < minHeight) {
        newHeight = minHeight;
        newWidth = static_cast<int>(newHeight * aspectRatio);
    }
    if (newHeight > maxHeight) {
        newHeight = maxHeight;
        newWidth = static_cast<int>(newHeight * aspectRatio);
    }

    setFixedSize(newWidth, newHeight);
    setPixmap(normalPixmap.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    return QSize(newWidth, newHeight);
}
