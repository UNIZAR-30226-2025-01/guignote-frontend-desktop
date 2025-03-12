#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QDialog>
#include <QMovie>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>

/**
 * @file loadingwindow.h
 * @brief Declaración de la clase LoadingWindow.
 *
 * Esta clase representa la pantalla de carga de la aplicación.
 * Muestra el gif "carga.gif" a pantalla completa durante 5 segundos y,
 * tras un efecto de desvanecimiento, transfiere el control a la ventana del menú (MenuWindow).
 */
class LoadingWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la pantalla de carga.
     * @param parent Widget padre.
     */
    explicit LoadingWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de la pantalla de carga.
     */
    ~LoadingWindow();

protected:
    /**
     * @brief Evento que se ejecuta al mostrar la ventana.
     *
     * Inicia la reproducción del gif y programa la transición a la ventana del menú.
     *
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;


private slots:
    /**
     * @brief Inicia la animación de desvanecimiento de la pantalla de carga.
     */
    void startFadeOut();

    /**
     * @brief Slot que se ejecuta al finalizar la animación de desvanecimiento.
     *
     * Abre la ventana del menú y cierra la pantalla de carga.
     */
    void onFadeOutFinished();

private:
    QLabel *gifLabel;                ///< Etiqueta para mostrar el gif.
    QMovie *loadingMovie;            ///< Objeto QMovie para reproducir el gif.
    QTimer *displayTimer;            ///< Timer para controlar la duración de la pantalla de carga.
    QPropertyAnimation *fadeAnimation; ///< Animación para desvanecer la pantalla.
    bool fadeOutStarted = false;    ///< Flag para saber si el fade out ha empezado

};

#endif // LOADINGWINDOW_H
