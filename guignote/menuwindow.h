/**
 * @file menuwindow.h
 * @brief Declaración de la clase MenuWindow.
 *
 * La clase MenuWindow representa la ventana principal del menú de la aplicación.
 * En ella se disponen elementos visuales y botones para seleccionar diferentes modos de juego,
 * además de adornos decorativos que se reajustan al cambiar el tamaño de la ventana.
 */

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
#include "icon.h"

/**
 * @class MenuWindow
 * @brief Ventana principal del menú.
 *
 * MenuWindow hereda de QWidget y define la interfaz principal del menú de la aplicación.
 * Contiene elementos gráficos como etiquetas, botones y adornos decorativos que se reposicionan
 * al redimensionar la ventana.
 */
class MenuWindow : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor de MenuWindow.
     *
     * Inicializa la ventana del menú, carga la interfaz definida en el archivo .ui y configura
     * los elementos gráficos y botones.
     *
     * @param parent Puntero al widget padre, por defecto nullptr.
     */
    explicit MenuWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de MenuWindow.
     *
     * Libera los recursos asociados a la ventana del menú.
     */
    ~MenuWindow();

protected:
    /**
     * @brief Evento de redimensionamiento.
     *
     * Se invoca automáticamente cuando la ventana cambia de tamaño. Reposiciona los adornos
     * decorativos para mantener la coherencia visual.
     *
     * @param event Evento de tipo QResizeEvent.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief Reposiciona los adornos decorativos.
     *
     * Calcula y actualiza la posición de los adornos en las esquinas de la ventana en función
     * del tamaño actual del widget central.
     */
    void repositionOrnaments();
    void repositionImageButtons();
    void repositionBars();
    void repositionIcons();
    void repositionTopIcons();

    Ui::MenuWindow *ui;
    QSize ornamentSize;
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QFrame *bottomBar;
    QFrame *topBar;
    Icon *settings;
    Icon *friends;
    Icon *exit;
    Icon *inventory;
    ImageButton *boton1v1;
    ImageButton *boton2v2;
};

#endif // MENUWINDOW_H
