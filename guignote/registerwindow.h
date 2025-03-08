/**
 * @file registerwindow.h
 * @brief Declaración de la clase RegisterWindow.
 *
 * Esta clase representa la ventana de registro en la aplicación. Permite a los usuarios
 * crear una nueva cuenta y emite una señal para cambiar a la ventana de inicio de sesión en caso de que ya tengan una cuenta.
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
     *
     * Inicializa la ventana de registro y configura la interfaz de usuario.
     *
     * @param parent Puntero al widget padre, por defecto nullptr.
     */
    explicit RegisterWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de RegisterWindow.
     *
     * Libera los recursos utilizados por la ventana de registro.
     */
    ~RegisterWindow();

signals:
    /**
     * @brief Señal emitida cuando el usuario desea iniciar sesión.
     *
     * Esta señal se emite cuando el usuario pulsa “¿Ya tienes cuenta? Inicia sesión”,
     * permitiendo cambiar a la ventana de inicio de sesión.
     */
    void openLoginRequested();

protected:
    /**
     * @brief Evento que se ejecuta al mostrar la ventana.
     *
     * Permite realizar acciones personalizadas al momento de mostrar la ventana,
     * como la creación de un overlay para centrar la atención en el diálogo.
     *
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento que se ejecuta al cerrar la ventana.
     *
     * Permite realizar tareas de limpieza, como la eliminación de overlays o la remoción
     * de filtros de eventos instalados en el widget padre.
     *
     * @param event Evento de tipo QCloseEvent.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Filtra eventos del widget y sus hijos.
     *
     * Permite interceptar eventos específicos (como redimensionamientos o movimientos)
     * para mantener la coherencia visual y funcional de la interfaz.
     *
     * @param watched Objeto que recibe el evento.
     * @param event Evento que ocurre.
     * @return true si el evento es manejado, false en caso contrario.
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
    QWidget *backgroundOverlay; /**< Overlay semitransparente para resaltar el diálogo de registro. */

    // Aquí irán todos los widgets y lógica de registro
};

#endif // REGISTERWINDOW_H
