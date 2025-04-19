#ifndef DECK_H
#define DECK_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QDebug>
#include "carta.h"

class Deck : public QWidget
{
    Q_OBJECT

public:
    explicit Deck(Carta* triunfo, int skin, int cardSize, QWidget *parent = nullptr, QString token = "");
    void actualizarVisual();
    void setTriunfo(Carta* nuevaTriunfo);
    void setNum(int n);
    void cartaRobada();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QString token;
    Carta* triunfo;
    int skin;
    int num;

    QLabel* fondo;
    QLabel* contador;

    void setImagenFondo(const QPixmap &pixmap, int alturaDeseada);
};

#endif // DECK_H
