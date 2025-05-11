/**
 * @file recoverpasswordwindow.h
 * @brief Declaración de la clase RecoverPasswordWindow, diálogo de recuperación de contraseña.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase RecoverPasswordWindow proporciona una interfaz modal para que el usuario
 * inicie el proceso de recuperación de contraseña. Añade un overlay semitransparente
 * sobre el widget padre para mejorar la experiencia visual.
 */

#ifndef RECOVERPASSWORDWINDOW_H
#define RECOVERPASSWORDWINDOW_H

#include <QDialog>

/**
 * @class RecoverPasswordWindow
 * @brief Diálogo para recuperación de contraseña.
 *
 * Centra el diálogo sobre el widget padre y añade un fondo oscuro para mejorar la
 * visibilidad. También gestiona el reposicionamiento automático si el padre se mueve
 * o cambia de tamaño.
 */
class RecoverPasswordWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de RecoverPasswordWindow.
     * @param parent Puntero al widget padre (por defecto nullptr).
     */
    explicit RecoverPasswordWindow(QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~RecoverPasswordWindow();

protected:
    /**
     * @brief Evento que se dispara al mostrar el diálogo.
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento que se dispara al cerrar el diálogo.
     * @param event Evento de tipo QCloseEvent.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Filtra eventos en el widget padre para manejar movimientos/redimensiones.
     * @param watched Objeto observado.
     * @param event Evento detectado.
     * @return true si el evento se procesa.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief Limpia recursos (overlay y filtros) antes de cerrar el diálogo.
     */
    void reject() override;

private:
    QWidget *backgroundOverlay; ///< Overlay semitransparente que oscurece el fondo del widget padre.
};

#endif // RECOVERPASSWORDWINDOW_H
