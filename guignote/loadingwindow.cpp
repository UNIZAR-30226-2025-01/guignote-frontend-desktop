/**
 * @file loadingwindow.cpp
 * @brief Implementación de la clase LoadingWindow.
 *
 * Esta clase muestra una pantalla de carga a pantalla completa con el gif "carga.gif".
 * La pantalla se muestra durante 5 segundos y luego se desvanece, pasando a la ventana del menú (MenuWindow).
 */

#include "loadingwindow.h"
#include "menuwindow.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QMainWindow>
#include <QDebug>

LoadingWindow::LoadingWindow(QWidget *parent)
    : QDialog(parent),
    gifLabel(new QLabel(this)),
    loadingMovie(new QMovie(":/video/carga.gif")),
    displayTimer(new QTimer(this)),
    fadeAnimation(nullptr),
    fadeOutStarted(false)
{
    if (!loadingMovie->isValid()) {
        qWarning() << "El gif no se pudo cargar. Verifica la ruta.";
    }

    gifLabel->setScaledContents(true);

    // Configuración de la ventana: sin bordes, a pantalla completa y fondo negro.
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setModal(true);
    setStyleSheet("background-color: black;");

    // Configuración del layout principal.
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(gifLabel);
    gifLabel->setAlignment(Qt::AlignCenter);

    // Configuración del gif.
    gifLabel->setMovie(loadingMovie);
    loadingMovie->start();

    // Configurar el timer para la duración de la pantalla de carga (5 segundos).
    displayTimer->setSingleShot(true);
    connect(displayTimer, &QTimer::timeout, this, &LoadingWindow::startFadeOut);

}

LoadingWindow::~LoadingWindow()
{
    if (fadeAnimation) {
        delete fadeAnimation;
    }
}

/**
 * @brief Evento que se ejecuta al mostrar la ventana.
 *
 * Inicia el gif y programa la transición a la ventana del menú.
 *
 * @param event Evento de tipo QShowEvent.
 */
void LoadingWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // Si hay widget padre, se ajusta al área completa del padre.
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
    } else {
        // Si no hay widget padre, se ajusta a la geometría de la pantalla disponible.
        setGeometry(QGuiApplication::primaryScreen()->availableGeometry());
    }
    if (loadingMovie) {
        loadingMovie->start();
    }
    // Inicia el timer solo si no se ha iniciado ya el fade out
    if (!fadeOutStarted) {
        displayTimer->start(3500);
    }
}




/**
 * @brief Inicia la animación de desvanecimiento de la pantalla de carga.
 */
void LoadingWindow::startFadeOut() {
    if (fadeOutStarted)
        return;
    fadeOutStarted = true;
    this->setWindowOpacity(1.0);  // Asegurarse de que empieza en opacidad completa.
    fadeAnimation = new QPropertyAnimation(this, "windowOpacity");
    fadeAnimation->setDuration(1000); // Duración de 1 segundo.
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    connect(fadeAnimation, &QPropertyAnimation::finished, this, &LoadingWindow::onFadeOutFinished);
    fadeAnimation->start();
}


/**
 * @brief Slot que se ejecuta al finalizar la animación de desvanecimiento.
 *
 * Abre la ventana del menú y cierra la pantalla de carga.
 */
void LoadingWindow::onFadeOutFinished() {
    if (loadingMovie) {
        loadingMovie->stop();
    }
    // Si LoadingWindow tiene un padre y éste es un QMainWindow, se reemplaza su widget central.
    QMainWindow *mainWin = qobject_cast<QMainWindow*>(parentWidget());
    if (mainWin) {
        MenuWindow *menuWin = new MenuWindow(mainWin);
        mainWin->setCentralWidget(menuWin);
    } else {
        // Si no se tiene un QMainWindow, se crea MenuWindow con la misma geometría.
        MenuWindow *menuWin = new MenuWindow(nullptr);
        menuWin->setGeometry(this->geometry());
        menuWin->show();
    }

    if (fadeAnimation) {
        fadeAnimation->deleteLater();
        fadeAnimation = nullptr;
    }
    // En lugar de cerrar la ventana padre, se cierra solamente la pantalla de carga.
    this->close();
}



void LoadingWindow::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);  // Llama a la implementación base.
    gifLabel->setFixedSize(this->size());  // Ajusta el label al tamaño de la ventana.
}




