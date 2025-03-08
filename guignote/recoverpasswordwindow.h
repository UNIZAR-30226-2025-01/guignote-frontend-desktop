#ifndef RECOVERPASSWORDWINDOW_H
#define RECOVERPASSWORDWINDOW_H

#include <QDialog>

// Clase RecoverPasswordWindow: Diálogo para la recuperación de contraseña que se centra
// automáticamente en el widget padre y actualiza su posición al mover o redimensionar dicho widget.
class RecoverPasswordWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de RecoverPasswordWindow.
     * @param parent Puntero al widget padre (por defecto nullptr).
     */
    explicit RecoverPasswordWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de RecoverPasswordWindow.
     */
    ~RecoverPasswordWindow();

protected:
    /**
     * @brief Evento que se dispara al mostrar el diálogo.
     * Se utiliza para crear un overlay semitransparente en el widget padre y centrar el diálogo.
     * @param event Puntero al evento de muestra.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento que se dispara al cerrar el diálogo.
     * Se utiliza para eliminar el overlay y remover el filtro de eventos instalado en el widget padre.
     * @param event Puntero al evento de cierre.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Filtro de eventos para detectar cambios en el widget padre.
     * Reposiciona el diálogo y actualiza el overlay cuando el widget padre se mueve o se redimensiona.
     * @param watched Objeto observado.
     * @param event Evento detectado.
     * @return true si el evento es procesado, de lo contrario false.
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
    QWidget *backgroundOverlay; ///< Overlay semitransparente que oscurece el fondo del widget padre.
};

#endif // RECOVERPASSWORDWINDOW_H
