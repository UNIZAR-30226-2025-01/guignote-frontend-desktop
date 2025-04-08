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
    explicit Deck(Carta* triunfo, int skin, int cardSize, QWidget *parent = nullptr);
    void actualizarVisual();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Carta* triunfo;
    int skin;
    int num;

    QLabel* fondo;
    QLabel* contador;

    void setImagenFondo(const QPixmap &pixmap, int alturaDeseada);
};

#endif // DECK_H
