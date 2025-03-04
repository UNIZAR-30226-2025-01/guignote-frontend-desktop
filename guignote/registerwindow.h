#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>

/**
 * @brief La clase RegisterWindow representa el diálogo de "Crear Cuenta".
 *
 * Este diálogo se centra automáticamente en su ventana padre y,
 * al mostrarse, oscurece el fondo para resaltar el popup.
 */
class RegisterWindow : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Constructor del diálogo de registro.
     * @param parent Widget padre (normalmente la ventana principal).
     */
    explicit RegisterWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~RegisterWindow();

protected:
    /**
     * @brief Se interceptan eventos del padre para detectar redimensionamientos.
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

#endif // REGISTERWINDOW_H
