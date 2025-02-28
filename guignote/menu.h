#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QLabel>
#include <QSize>
#include "ui_menu.h"  // ← ¡Esto debe estar aquí!

class menu : public QWidget {
    Q_OBJECT

public:
    explicit menu(QWidget *parent = nullptr);
    ~menu();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void repositionOrnaments();

    Ui::menu *ui;  // ¡Ahora sí Qt lo reconocerá!
    QSize ornamentSize;
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QLabel *topBar; // Nueva barra superior
};

#endif // MENU_H
