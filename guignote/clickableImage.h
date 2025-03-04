#ifndef CLICKABLEIMAGE_H
#define CLICKABLEIMAGE_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class ClickableImage : public QLabel {
    Q_OBJECT

public:
    explicit ClickableImage(QWidget *parent = nullptr);

    // Método para cargar una imagen
    void setImage(const QString &imagePath, int width = 100, int height = 100);

signals:
    void clicked(); // Señal emitida cuando la imagen es clickeada

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // CLICKABLEIMAGE_H
