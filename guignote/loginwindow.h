#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

/**
 * @brief La clase LoginWindow representa el diálogo de "Iniciar Sesión".
 *
 * Este diálogo se centra automáticamente en su ventana padre y,
 * al mostrarse, oscurece el fondo de dicha ventana para resaltar el popup.
 */
class LoginWindow : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Constructor del diálogo de inicio de sesión.
     * @param parent Widget padre (normalmente la ventana principal).
     */
    explicit LoginWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~LoginWindow();

protected:
    /**
     * @brief Se interceptan eventos del padre para detectar redimensionamientos.
     *
     * Permite reposicionar el diálogo y ajustar el overlay cuando el padre cambia de tamaño.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief Se sobreescribe showEvent para crear el overlay y centrar el diálogo.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Se sobreescribe closeEvent para remover el overlay y el filtro de eventos.
     */
    void closeEvent(QCloseEvent *event) override;

private:
    /// Widget que actúa como overlay semitransparente para oscurecer el fondo.
    QWidget *backgroundOverlay;
};

#endif // LOGINWINDOW_H
