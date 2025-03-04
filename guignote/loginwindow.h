/**
 * @file loginwindow.h
 * @brief Declaración de la clase LoginWindow.
 *
 * Esta clase representa la ventana de inicio de sesión de la aplicación.
 * Es un diálogo modal que permite al usuario iniciar sesión y proporciona
 * un mecanismo para redirigir a la ventana de registro en caso de que el usuario
 * no tenga cuenta.
 */

#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

/**
 * @class LoginWindow
 * @brief Ventana de inicio de sesión.
 *
 * La clase LoginWindow hereda de QDialog y representa una ventana emergente
 * en la que los usuarios pueden ingresar sus credenciales para autenticarse.
 * También permite la navegación hacia la ventana de registro a través de una señal.
 */
class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de LoginWindow.
     *
     * Inicializa la ventana de inicio de sesión.
     *
     * @param parent Puntero al widget padre, por defecto nullptr.
     */
    explicit LoginWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de LoginWindow.
     *
     * Libera los recursos utilizados por la ventana de inicio de sesión.
     */
    ~LoginWindow();

signals:
    /**
     * @brief Señal emitida cuando el usuario desea abrir la ventana de registro.
     *
     * Esta señal se emite cuando el usuario pulsa en la opción
     * “¿No tienes cuenta? Crea una”.
     */
    void openRegisterRequested();

protected:
    /**
     * @brief Evento que se dispara cuando la ventana se muestra.
     *
     * Se puede sobrescribir para personalizar el comportamiento al mostrar
     * la ventana de inicio de sesión.
     *
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento que se dispara cuando la ventana se cierra.
     *
     * Se puede sobrescribir para realizar tareas de limpieza antes de cerrar la ventana.
     *
     * @param event Evento de tipo QCloseEvent.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Filtra eventos de la ventana y sus hijos.
     *
     * Este método se usa para interceptar eventos específicos en la interfaz,
     * como clics o teclas presionadas, para modificar el comportamiento predeterminado.
     *
     * @param watched Objeto que está recibiendo el evento.
     * @param event Evento que está ocurriendo.
     * @return true si el evento es manejado, false en caso contrario.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *backgroundOverlay; /**< Fondo semi-transparente para resaltar el diálogo. */
};

#endif // LOGINWINDOW_H
