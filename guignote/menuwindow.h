#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QSize>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMainWindow>
#include "imagebutton.h"
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
    ImageButton *boton1v1;
    ImageButton *boton2v2;
};

#endif // MENUWINDOW_H
