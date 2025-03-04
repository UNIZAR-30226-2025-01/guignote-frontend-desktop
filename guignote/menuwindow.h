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
#include "ui_menuwindow.h"

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

    Ui::MenuWindow *ui;           /**< Puntero a la interfaz generada con Qt Designer. */
    QSize ornamentSize;           /**< Tamaño definido para los adornos decorativos. */
    QLabel *cornerTopLeft;        /**< Adorno en la esquina superior izquierda. */
    QLabel *cornerTopRight;       /**< Adorno en la esquina superior derecha. */
    QLabel *cornerBottomLeft;     /**< Adorno en la esquina inferior izquierda. */
    QLabel *cornerBottomRight;    /**< Adorno en la esquina inferior derecha. */
    QLabel *topBar;               /**< Barra superior de la ventana (opcional). */
    QPushButton *button1v1;       /**< Botón para seleccionar el modo de juego 1 contra 1. */
    QPushButton *button2v2;       /**< Botón para seleccionar el modo de juego 2 contra 2. */
    QLabel *cartasAtras;          /**< Etiqueta para mostrar las cartas de la parte trasera. */
    QLabel *cartasDelante;        /**< Etiqueta para mostrar las cartas de la parte delantera. */
};

#endif // MENUWINDOW_H
