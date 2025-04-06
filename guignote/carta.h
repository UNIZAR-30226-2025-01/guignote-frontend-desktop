#ifndef CARTA_H
#define CARTA_H

#include <QLabel>
#include <QPixmap>
#include <QPoint>

// ✅ Solo una declaración adelantada
class Mano;

class Carta : public QLabel
{
    Q_OBJECT

public:
    explicit Carta(QWidget *parent = nullptr, QString num = "0", QString suit = "", int h = 100, int skin = 0);

    void añadirAMano(Mano* mano, int id);  // puntero
    void eliminarDeMano();
    void setLock(bool lock);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool locked;
    QPixmap imagen;
    QPoint offsetArrastre;
    bool arrastrando;
    int ID;
    Mano* mano;  // ✅ puntero a Mano
    void setImagen(const QPixmap &pixmap, int h);
    QPixmap selectPixmap(QString num, QString suit, int skin = 0);
};

#endif // CARTA_H
