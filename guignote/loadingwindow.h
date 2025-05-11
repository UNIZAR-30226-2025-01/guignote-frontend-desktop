/**
 * @file loadingwindow.h
 * @brief Declaración de la clase LoadingWindow, pantalla de carga de la aplicación.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Esta clase representa la pantalla de carga inicial que muestra una animación GIF
 * ("carga.gif") a pantalla completa durante unos segundos, seguido de un efecto
 * de desvanecimiento que da paso a la ventana principal del menú (MenuWindow).
 */

#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QDialog>
#include <QMovie>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>

/**
 * @class LoadingWindow
 * @brief Ventana que muestra una animación de carga antes de mostrar el menú principal.
 */
class LoadingWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la pantalla de carga.
     * @param userKey Clave del usuario para pasar al menú principal.
     * @param parent Widget padre.
     */
    explicit LoadingWindow(const QString &userKey, QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~LoadingWindow();

protected:
    /**
     * @brief Evento que se ejecuta al mostrar la ventana.
     *
     * Inicia la reproducción del gif y programa la transición a la ventana del menú.
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief Evento de redimensionamiento de la ventana.
     * @param event Evento de tipo QResizeEvent.
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /**
     * @brief Inicia la animación de desvanecimiento de la pantalla de carga.
     * @param userKey Clave del usuario.
     */
    void startFadeOut(const QString &userKey);

    /**
     * @brief Slot que se ejecuta al finalizar la animación de desvanecimiento.
     *
     * Abre la ventana del menú y cierra la pantalla de carga.
     * @param userKey Clave del usuario.
     */
    void onFadeOutFinished(const QString &userKey);

private:
    QString userKey;                      ///< Clave del usuario autenticado.

    QLabel *gifLabel;                     ///< Etiqueta para mostrar el gif.
    QMovie *loadingMovie;                 ///< Objeto QMovie para reproducir el gif.
    QTimer *displayTimer;                 ///< Temporizador para controlar duración.
    QPropertyAnimation *fadeAnimation;    ///< Animación para desvanecer la pantalla.
    bool fadeOutStarted = false;          ///< Bandera para evitar múltiples fade outs.
};

#endif // LOADINGWINDOW_H
