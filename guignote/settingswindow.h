#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>

// Declaraciones adelantadas de clases para evitar incluir sus cabeceras
class QListWidget;
class QStackedWidget;
class QRadioButton;
class QPushButton;

/**
 * @brief Ventana de configuración de la aplicación.
 *
 * SettingsWindow permite al usuario modificar opciones de visualización,
 * como el modo gráfico (ventana o pantalla completa). La interfaz se compone de
 * un sidebar para la navegación y un área de contenido para las opciones.
 */
class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de SettingsWindow.
     * @param parent Puntero al widget padre, por defecto nullptr.
     */
    explicit SettingsWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de SettingsWindow.
     */
    ~SettingsWindow();

private:
    QListWidget *sidebar;           ///< Widget lateral para la navegación.
    QStackedWidget *stackedWidget;  ///< Área que muestra las páginas de configuración.
    QRadioButton *radioWindowed;    ///< Botón de opción para modo ventana.
    QRadioButton *radioFullscreen;  ///< Botón de opción para modo pantalla completa.
    QPushButton *closeButton;       ///< Botón para cerrar la ventana de configuración.

    /**
     * @brief Actualiza el modo gráfico de la aplicación.
     *
     * Cambia el modo de visualización del widget padre entre ventana y pantalla completa
     * según la opción seleccionada.
     */
    void updateGraphicsMode();
};

#endif // SETTINGSWINDOW_H
