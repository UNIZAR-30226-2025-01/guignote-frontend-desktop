#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QSize>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QPixmap>
#include <QTransform>
#include <QWindow>
#include "imagebutton.h"
#include "ui_menuwindow.h"
#include "clickableImage.h"

class MenuWindow : public QWidget {
    Q_OBJECT

public:
    explicit MenuWindow(QWidget *parent = nullptr);
    ~MenuWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void repositionOrnaments();
    void repositionImageButtons();
    void repositionBars();

    Ui::MenuWindow *ui;
    QSize ornamentSize;
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QFrame *bottomBar;
    QFrame *topBar;
    ClickableImage *settings;
    ClickableImage *friends;
    ImageButton *boton1v1;
    ImageButton *boton2v2;
};

#endif // MENUWINDOW_H
