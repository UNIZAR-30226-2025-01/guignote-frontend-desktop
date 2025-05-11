/**
 * @file registerwindow.h
 * @brief Declaración de la clase RegisterWindow, interfaz de registro de usuario.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Esta clase representa la ventana de registro en la aplicación. Permite a los usuarios
 * crear una nueva cuenta y emite una señal para cambiar a la ventana de inicio de sesión
 * en caso de que ya tengan una cuenta.
 */

#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>

/**
 * @class RegisterWindow
 * @brief Ventana de registro de usuario.
 *
 * La clase RegisterWindow hereda de QDialog y proporciona la interfaz para la creación de una nueva cuenta.
 * Además, incluye la lógica para gestionar eventos propios de la ventana, como la creación de un overlay
 * y el filtrado de eventos para mantener la coherencia visual.
 */
class RegisterWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de RegisterWindow.
     * @param parent Puntero al widget padre, por defecto nullptr.
     */
    explicit RegisterWindow(QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~RegisterWindow();

signals:
    /**
     * @brief Señal emitida cuando el usuario desea iniciar sesión.
     */
    void openLoginRequested();

protected:
    /**
     * @brief Evento que se ejecuta al mostrar la ventana.
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento que se ejecuta al cerrar la ventana.
     * @param event Evento de tipo QCloseEvent.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Filtra eventos del widget y sus hijos.
     * @param watched Objeto observado.
     * @param event Evento interceptado.
     * @return true si el evento fue manejado.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief Limpia recursos adicionales antes de cerrar el diálogo.
     */
    void reject() override;

private:
    QWidget *backgroundOverlay; ///< Overlay semitransparente para resaltar el diálogo de registro.

    // Aquí irán todos los widgets y lógica de registro
};

#endif // REGISTERWINDOW_H
