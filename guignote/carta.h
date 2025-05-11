#ifndef CARTA_H
#define CARTA_H

#include "orientacion.h"
#include <QString>
#include <QLabel>
#include <QEnterEvent>
#include <QEvent>

class Carta : public QLabel {
    Q_OBJECT
public:
    static int skin;

    Carta(QWidget *parent = nullptr);

    Carta(const QString& palo, const QString& valor, QWidget *parent = nullptr);

    void setPaloValor(const QString& palo, const QString& valor);

    QString getPalo() const;

    QString getValor() const;

    void setPosicion(int x, int y);

    void setOrientacion(Orientacion orientacion);

    bool interactuable = false;

signals:
    void cartaDobleClick(Carta* carta);

private:
    void cargarImagen();

    QPixmap img[2];
    QString palo, valor;
    int posX, posY;
    Orientacion orientacion;

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};

#endif // CARTA_H
