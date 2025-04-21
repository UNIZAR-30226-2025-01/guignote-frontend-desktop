// carta.h
#ifndef CARTA_H
#define CARTA_H

#include <QLabel>
#include <QPixmap>
#include <QPoint>

class Mano;
class GameWindow;

class Carta : public QLabel
{
    Q_OBJECT

public:
    explicit Carta(GameWindow *gw = nullptr,
                   QWidget *parent = nullptr,
                   const QString &num    = "0",
                   const QString &suit   = "",
                   int h                  = 100,
                   int skin               = 0,
                   bool faceUp            = true);

    void reveal();
    void hideFace();
    void añadirAMano(Mano* mano, int id);
    void eliminarDeMano();
    void setLock(bool lock);
    QPixmap getImagen() const;
    void setImagen(const QPixmap &pixmap, int h);

    QString idGlobal;  // identificador único ("num"+"suit")
    QString num;
    QString suit;
    QPixmap getOriginalPixmap() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPixmap selectPixmap(int skin) const;

    QPixmap frontPixmap;
    QPixmap backPixmap;
    QPixmap imagen;
    QPixmap pixmapOrig;

    bool isFaceUp;
    bool locked;
    bool arrastrando;
    QPoint offsetArrastre;
    int ID;
    Mano* mano;
};

#endif // CARTA_H
