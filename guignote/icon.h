#ifndef ICON_H
#define ICON_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class Icon : public QLabel {
    Q_OBJECT

public:
    explicit Icon(QWidget *parent = nullptr);

    // Método para cargar una imagen
    void setImage(const QString &imagePath, int width = 100, int height = 100);

signals:
    void clicked(); // Señal emitida cuando la imagen es clickeada

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // ICON_H
