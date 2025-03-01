#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QLabel>
#include <QSize>
#include <QPushButton>
#include "ui_menu.h"

class menu : public QWidget {
    Q_OBJECT

public:
    explicit menu(QWidget *parent = nullptr);
    ~menu();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void repositionOrnaments();

    Ui::menu *ui;
    QSize ornamentSize;
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QLabel *topBar;
    QPushButton *button1v1;
    QPushButton *button2v2;
    QLabel *cartasAtras;
    QLabel *cartasDelante;
};

#endif // MENU_H
