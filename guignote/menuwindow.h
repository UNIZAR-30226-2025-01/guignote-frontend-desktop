#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QSize>
#include <QPushButton>
#include "ui_menuwindow.h"

class MenuWindow : public QWidget {
    Q_OBJECT

public:
    explicit MenuWindow(QWidget *parent = nullptr);
    ~MenuWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void repositionOrnaments();

    Ui::MenuWindow *ui;
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

#endif // MENUWINDOW_H
