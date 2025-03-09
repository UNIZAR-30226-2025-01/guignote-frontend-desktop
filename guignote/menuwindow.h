#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QSize>
#include <QLabel>
#include <QFrame>
#include <QResizeEvent>

class ImageButton;
class Icon;

namespace Ui {
class MenuWindow;
}

class MenuWindow : public QWidget {
    Q_OBJECT
public:
    explicit MenuWindow(QWidget *parent = nullptr);
    ~MenuWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MenuWindow *ui;

    // Botones de modos de juego
    ImageButton *boton1v1;
    ImageButton *boton2v2;

    // Barras de la interfaz
    QFrame *bottomBar;
    QFrame *topBar;

    // Iconos de opciones (Settings, Friends, Exit, Inventory)
    Icon *settings;
    Icon *friends;
    Icon *exit;
    Icon *inventory;

    // Adornos decorativos en las esquinas
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QSize ornamentSize;

    // Métodos de reposicionamiento de elementos
    void repositionOrnaments();
    void repositionImageButtons();
    void repositionBars();
    void repositionIcons();
    void repositionTopIcons();
};

#endif // MENUWINDOW_H
