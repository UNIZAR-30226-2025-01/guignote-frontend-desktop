#ifndef CARTA_H
#define CARTA_H

#include <QLabel>
#include <QPixmap>
#include <QPoint>

class Carta : public QLabel
{
    Q_OBJECT

public:
    explicit Carta(QWidget *parent = nullptr, QString num ="0", QString suit ="", int h=100, int skin=0);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPixmap imagen;
    QPoint offsetArrastre;
    bool arrastrando;
    void setImagen(const QPixmap &pixmap, int h);
    QPixmap selectPixmap(QString num, QString suit, int skin);
};

#endif // CARTA_H
