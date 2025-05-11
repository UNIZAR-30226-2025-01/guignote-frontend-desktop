/**
 * @file loadingwindow.cpp
 * @brief Implementación de la clase LoadingWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de la ventana de carga a pantalla completa,
 * que muestra un GIF animado, gestiona el temporizador de visualización
 * y la animación de desvanecimiento antes de pasar a MenuWindow.
 */


#include "loadingwindow.h"
#include "menuwindow.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QShowEvent>
#include <QMainWindow>
#include <QDebug>

/**
 * @brief Constructor de la pantalla de carga.
 * @param userKey Clave del usuario para pasar a la siguiente ventana.
 * @param parent Widget padre opcional.
 *
 * Configura el diálogo sin bordes, a pantalla completa, inicializa
 * el GIF de carga, el temporizador de visualización y el efecto de opacidad.
 */

LoadingWindow::LoadingWindow(const QString &userKey, QWidget *parent)
    : QDialog(parent),
    userKey(userKey),
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

    // Configurar el timer para la duración de la pantalla de carga (3.5 segundos antes de iniciar el fade out).
    displayTimer->setSingleShot(true);
    connect(displayTimer, &QTimer::timeout, this, [this, userKey]() {
        this->startFadeOut(userKey);
    });

    // Configurar el efecto de opacidad.
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);
}

/**
 * @brief Destructor de LoadingWindow.
 *
 * Elimina la animación de desvanecimiento si existe.
 */

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
    // Inicia el timer solo si no se ha iniciado ya el fade out.
    if (!fadeOutStarted) {
        displayTimer->start(3500);
    }
}

/**
 * @brief Inicia la animación de desvanecimiento de la pantalla de carga.
 */
void LoadingWindow::startFadeOut(const QString &userKey) {
    if (fadeOutStarted)
        return;
    fadeOutStarted = true;

    // Obtener el efecto de opacidad aplicado.
    QGraphicsOpacityEffect *opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(this->graphicsEffect());
    if (!opacityEffect)
        return;

    // Configurar la animación sobre la propiedad "opacity" del efecto.
    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnimation->setDuration(1000); // Duración de 1 segundo.
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    connect(fadeAnimation, &QPropertyAnimation::finished, this, [=]() {
        this->onFadeOutFinished(userKey);
    });
    fadeAnimation->start();
}

/**
 * @brief Slot que se ejecuta al finalizar la animación de desvanecimiento.
 *
 * Abre la ventana del menú y cierra la pantalla de carga.
 */
void LoadingWindow::onFadeOutFinished(const QString &userKey) {
    if (loadingMovie) {
        loadingMovie->stop();
    }
    // Si LoadingWindow tiene un padre y éste es un QMainWindow, se reemplaza su widget central.
    QMainWindow *mainWin = qobject_cast<QMainWindow*>(parentWidget());
    if (mainWin) {
        MenuWindow *menuWin = new MenuWindow(userKey, mainWin);
        mainWin->setCentralWidget(menuWin);
        mainWin->showFullScreen();
    } else {
        // Si no se tiene un QMainWindow, se crea MenuWindow con la misma geometría.
        MenuWindow *menuWin = new MenuWindow(userKey);
        menuWin->setGeometry(this->geometry());
        menuWin->showFullScreen();
    }

    if (fadeAnimation) {
        fadeAnimation->deleteLater();
        fadeAnimation = nullptr;
    }
    // Cierra la pantalla de carga.
    this->close();
}

/**
 * @brief Evento que se ejecuta al cambiar el tamaño de la ventana.
 *
 * Ajusta el tamaño del label del gif al de la ventana.
 *
 * @param event Evento de tipo QResizeEvent.
 */
void LoadingWindow::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);  // Llama a la implementación base.
    gifLabel->setFixedSize(this->size());  // Ajusta el label al tamaño de la ventana.
}
