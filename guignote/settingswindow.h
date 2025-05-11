/**
 * @file settingswindow.h
 * @brief Declaración de la clase SettingsWindow, ventana de configuración de la aplicación.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase SettingsWindow permite al usuario modificar opciones de configuración,
 * como el modo de pantalla y los volúmenes de audio. Utiliza QSettings para guardar
 * y recuperar preferencias persistentes.
 */

#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>
#include <QSlider>

// Declaraciones adelantadas para evitar inclusiones innecesarias
class QListWidget;
class QStackedWidget;
class QRadioButton;
class QPushButton;

/**
 * @class SettingsWindow
 * @brief Ventana de configuración de la aplicación.
 *
 * Permite al usuario modificar parámetros visuales y de audio,
 * almacenándolos de forma persistente mediante QSettings.
 */
class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de SettingsWindow.
     * @param mainWindow Puntero a la ventana principal para aplicar cambios.
     * @param parent Widget padre, por defecto nullptr.
     * @param usr Nombre del usuario actual.
     */
    explicit SettingsWindow(QWidget *mainWindow, QWidget *parent = nullptr, QString usr = "");

    /** @brief Destructor. */
    ~SettingsWindow();

protected:
    /**
     * @brief Evento de cierre de la ventana.
     * @param event Evento de tipo QCloseEvent.
     *
     * Guarda los ajustes al cerrarse la ventana.
     */
    void closeEvent(QCloseEvent *event) override;

private:
    QWidget *mainWindowRef;         ///< Referencia a la ventana principal.
    QPushButton *closeButton;       ///< Botón para cerrar la ventana.
    QSlider *audioSlider;           ///< Control deslizante para volumen de música.
    QSlider *soundSlider;           ///< Control deslizante para volumen de efectos.
    QString usr;                    ///< Nombre de usuario asociado a la configuración.

    /** @brief Guarda los ajustes actuales en QSettings. */
    void saveSettings();

    /** @brief Carga los ajustes previamente guardados desde QSettings. */
    void loadSettings();
};

#endif // SETTINGSWINDOW_H
