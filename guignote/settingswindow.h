#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>
#include <QSlider>

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
    explicit SettingsWindow(QWidget *mainWindow, QWidget *parent = nullptr, QString usr="");

    /**
     * @brief Destructor de SettingsWindow.
     */
    ~SettingsWindow();

private:
    QWidget *mainWindowRef; // Guardar referencia al mainWindow
    QPushButton *closeButton;       ///< Botón para cerrar la ventana de configuración.
    QSlider *audioSlider;
    QSlider *soundSlider;
    QString usr;

    void saveSettings();  // Guarda los ajustes en QSettings
    void loadSettings();  // Carga los ajustes desde QSettings

protected:
    void closeEvent(QCloseEvent *event) override;  // Captura el evento de cierre para guardar los ajustes

};

#endif // SETTINGSWINDOW_H
