/**
 * @file loginwindow.h
 * @brief Declaración de la clase LoginWindow, ventana de inicio de sesión de usuario.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase LoginWindow gestiona la interfaz de autenticación inicial del usuario,
 * incluyendo validación de credenciales, navegación hacia el registro, y manejo visual
 * con un fondo oscurecido.
 */

#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

/**
 * @class LoginWindow
 * @brief Ventana de inicio de sesión.
 *
 * Permite al usuario ingresar sus credenciales, alternar la visibilidad de la contraseña,
 * y gestionar acciones relacionadas como la recuperación de contraseña o la solicitud
 * para abrir la ventana de registro.
 */
class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de inicio de sesión.
     * @param parent Widget padre, por defecto es nullptr.
     */
    explicit LoginWindow(QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~LoginWindow();

signals:
    /**
     * @brief Señal para solicitar la apertura de la ventana de registro.
     */
    void openRegisterRequested();

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
     * @brief Filtra los eventos enviados al widget padre.
     * @param watched Objeto que recibe el evento.
     * @param event Evento que se produce.
     * @return true si el evento se procesa, false en caso contrario.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief Sobrescribe el método reject() para limpiar recursos adicionales.
     */
    void reject() override;

private:
    /**
     * @brief Ejecutado cuando el inicio de sesión es exitoso.
     * @param userKey Clave del usuario autenticado.
     */
    void loginSucceeded(const QString &userKey);

    QWidget *backgroundOverlay; ///< Overlay para enfocar la atención en el diálogo.
};

#endif // LOGINWINDOW_H
