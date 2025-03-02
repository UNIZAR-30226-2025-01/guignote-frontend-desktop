#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QEnterEvent>  // Agregar para Qt 6

class ImageButton : public QLabel {
    Q_OBJECT

public:
    explicit ImageButton(const QString &imagePath, const QString &text, QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void enterEvent(QEnterEvent *event) override;  // Cambiado de QEvent a QEnterEvent
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    QPixmap generateDarkenedPixmap(const QPixmap &source);

private:
    QPixmap normalPixmap;
    QLabel *textLabel;
    QPixmap darkenedPixmap; // Imagen oscurecida

};

#endif // IMAGEBUTTON_H
