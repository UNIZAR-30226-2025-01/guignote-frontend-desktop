#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

/**
 * @brief Ventana de inicio de sesión.
 *
 * La clase LoginWindow representa la ventana de inicio de sesión de la aplicación.
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

    /**
     * @brief Destructor de la ventana de inicio de sesión.
     */
    ~LoginWindow();

signals:
    /**
     * @brief Señal para solicitar la apertura de la ventana de registro.
     *
     * Esta señal se emite cuando el usuario indica que aún no tiene cuenta y desea crear una.
     */
    void openRegisterRequested();

protected:
    /**
     * @brief Evento que se ejecuta al mostrar la ventana.
     * @param event Evento de tipo QShowEvent.
     *
     * Se utiliza para crear un overlay semitransparente en el widget padre y centrar el diálogo.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento que se ejecuta al cerrar la ventana.
     * @param event Evento de tipo QCloseEvent.
     *
     * Se encarga de eliminar el overlay y remover el filtro de eventos instalado en el widget padre.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Filtra los eventos enviados al widget padre.
     * @param watched Objeto que recibe el evento.
     * @param event Evento que se produce.
     * @return true si el evento se procesa, false en caso contrario.
     *
     * Este método se utiliza para actualizar la posición del diálogo y del overlay cuando
     * el widget padre se mueve o redimensiona.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief Sobrescribe el método reject() para limpiar recursos adicionales.
     *
     * Este método se encarga de realizar tareas de limpieza específicas antes de cerrar
     * el diálogo de inicio de sesión. En particular, elimina el filtro de eventos instalado
     * en el widget padre y programa la eliminación del overlay semitransparente (fondo oscuro)
     * que se aplicó al widget padre para enfocar la atención en el diálogo.
     *
     * Estas acciones aseguran que, al cerrarse el diálogo (por ejemplo, al presionar Escape),
     * no queden recursos residuales que bloqueen la interacción con la ventana principal.
     */
    void reject() override;

private:
    void loginSucceeded(const QString &userKey);
    QWidget *backgroundOverlay; ///< Overlay para enfocar la atención en el diálogo.
};

#endif // LOGINWINDOW_H
