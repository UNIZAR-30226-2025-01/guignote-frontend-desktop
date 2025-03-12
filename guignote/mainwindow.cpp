/**
 * @file mainwindow.cpp
 * @brief Implementación de la clase MainWindow.
 *
 * Esta clase representa la ventana principal de la aplicación. Se encarga de configurar la
 * interfaz gráfica de la pantalla de inicio, estableciendo el fondo, la disposición de widgets,
 * efectos visuales y la interacción con otras ventanas como LoginWindow, RegisterWindow y SettingsWindow.
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Otras ventanas utilizadas en la aplicación
#include "loginwindow.h"
#include "registerwindow.h"
#include "settingswindow.h" // Opcional, si se utiliza
#include "icon.h"

// Inclusión de librerías de Qt para gestionar layouts, widgets y efectos visuales
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QTransform>
#include <QDialog>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QColor>
#include <QImage>

/**
 * @brief Constructor de MainWindow.
 *
 * Inicializa la interfaz de usuario, establece el tamaño y fondo de la ventana, y configura
 * la disposición de widgets y efectos visuales. Además, define la lógica para abrir otras ventanas
 * (LoginWindow, RegisterWindow y SettingsWindow) y añade elementos decorativos en las esquinas.
 *
 * @param parent Widget padre, por defecto nullptr.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , cornerTopLeft(nullptr)
    , cornerTopRight(nullptr)
    , cornerBottomLeft(nullptr)
    , cornerBottomRight(nullptr)
{
    // Inicializa la interfaz generada con Qt Designer
    ui->setupUi(this);
    setWindowTitle("Inicio");

    // Configuración de tamaños mínimo, máximo y tamaño inicial de la ventana
    setMinimumSize(1090, 600);
    setMaximumSize(1920, 1080);
    resize(1090, 600);

    // Asegurar que exista un widget central
    if (!ui->centralwidget) {
        QWidget *central = new QWidget(this);
        setCentralWidget(central);
    }

    // Aplicar un fondo con gradiente radial a la ventana
    this->setStyleSheet(
        "QWidget {"
        "  background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
        "  fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
        "}"
        );

    // Creación de una caja central (marco) con fondo y bordes redondeados
    QFrame *centralBox = new QFrame(ui->centralwidget);
    centralBox->setStyleSheet(
        "QFrame {"
        "  background-color: #171718;"
        "  border-radius: 5px;"
        "  padding: 20px;"
        "}"
        );
    centralBox->setFixedSize(450, 450);

    // Aplicación de un efecto de sombra al marco central
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(centralBox);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(4, 4);
    centralBox->setGraphicsEffect(shadow);

    // Cargar una fuente personalizada para el título, mostrando una advertencia en caso de fallo
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    if (fontId == -1) {
        qWarning() << "No se pudo cargar la fuente personalizada.";
    }

    // Layout interno del marco central, centrado verticalmente
    QVBoxLayout *boxLayout = new QVBoxLayout(centralBox);
    boxLayout->setAlignment(Qt::AlignCenter);

    // Creación y configuración del título de la aplicación
    QLabel *titleLabel = new QLabel("SOTA, CABALLO Y REY", centralBox);
    QFont titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).value(0), 32);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    boxLayout->addWidget(titleLabel);

    // Configuración del logo de la aplicación
    QLabel *logoLabel = new QLabel(centralBox);
    QPixmap logoPixmap(":/images/app_logo_white.png");
    logoLabel->setScaledContents(false);
    logoLabel->setFixedSize(150, 150);
    QPixmap scaledLogo = logoPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logoLabel->setPixmap(scaledLogo);
    boxLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    boxLayout->addSpacing(50);

    // Creación de los botones "Iniciar Sesión" y "Crear Cuenta" con estilo personalizado
    QPushButton *loginButton = new QPushButton("Iniciar Sesión", centralBox);
    QPushButton *registerButton = new QPushButton("Crear Cuenta", centralBox);

    QString buttonStyle =
        "QPushButton {"
        "  background-color: #c2c2c3;"
        "  color: #171718;"
        "  border-radius: 10px;"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "  padding: 12px 25px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #9b9b9b;"
        "}";
    loginButton->setStyleSheet(buttonStyle);
    registerButton->setStyleSheet(buttonStyle);
    loginButton->setFixedSize(250, 50);
    registerButton->setFixedSize(250, 50);

    // Conexión de los botones con sus respectivos slots para abrir las ventanas de Login y Registro
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::openLoginWindow);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::openRegisterWindow);

    // Layout vertical para agrupar los botones de login y registro
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setSpacing(30);
    buttonLayout->addWidget(loginButton, 0, Qt::AlignCenter);
    buttonLayout->addWidget(registerButton, 0, Qt::AlignCenter);
    boxLayout->addLayout(buttonLayout);

    // Botones inferiores: preferencias y salir
    Icon *preferencesButton = new Icon(this);
    preferencesButton->setImage(":/icons/settings.png", 80, 80);
    connect(preferencesButton, &Icon::clicked, [=]() {
        SettingsWindow *settingsWin = new SettingsWindow(this,this);
        settingsWin->setModal(true);
        settingsWin->show();
    });

    // ICONO SALIDA

    Icon *exitIconButton = new Icon(this);
    exitIconButton->setImage(":/icons/door.png", 90, 90);

    connect(exitIconButton, &Icon::clicked, [=]() {
        exitIconButton->setImage(":/icons/darkeneddoor.png", 90, 90);
    });

    connect(exitIconButton, &Icon::clicked, [=]() {
        QDialog *confirmDialog = new QDialog(this);
        confirmDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        confirmDialog->setModal(true);
        confirmDialog->setFixedSize(300,150);
        confirmDialog->setStyleSheet(
            "QDialog {"
            "  background-color: #171718;"
            "  border-radius: 5px;"
            "  padding: 20px;"
            "}"
            );

        QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(confirmDialog);
        dialogShadow->setBlurRadius(10);
        dialogShadow->setColor(QColor(0, 0, 0, 80));
        dialogShadow->setOffset(4, 4);
        confirmDialog->setGraphicsEffect(dialogShadow);

        QVBoxLayout *dialogLayout = new QVBoxLayout(confirmDialog);
        QLabel *confirmLabel = new QLabel("¿Está seguro que desea salir?", confirmDialog);
        confirmLabel->setStyleSheet("QFrame { background-color: #171718; border-radius: 5px; }");
        confirmLabel->setAlignment(Qt::AlignCenter);
        dialogLayout->addWidget(confirmLabel);

        QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
        QPushButton *yesButton = new QPushButton("Sí", confirmDialog);
        QPushButton *noButton = new QPushButton("No", confirmDialog);
        yesButton->setStyleSheet(buttonStyle);
        noButton->setStyleSheet(buttonStyle);
        yesButton->setFixedSize(100,40);
        noButton->setFixedSize(100,40);
        dialogButtonLayout->addWidget(yesButton);
        dialogButtonLayout->addWidget(noButton);
        dialogLayout->addLayout(dialogButtonLayout);

        connect(yesButton, &QPushButton::clicked, [=]() {
            qApp->quit();
        });
        connect(noButton, &QPushButton::clicked, [=]() {
            confirmDialog->close();
        });
        connect(confirmDialog, &QDialog::finished, [=](int) {
            exitIconButton->setImage(":/icons/door.png", 90, 90);
        });

        confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        confirmDialog->show();
    });

    QHBoxLayout *bottomButtonLayout = new QHBoxLayout();
    bottomButtonLayout->addWidget(preferencesButton, 0, Qt::AlignLeft);
    bottomButtonLayout->addStretch();
    bottomButtonLayout->addWidget(exitIconButton, 0, Qt::AlignRight);
    boxLayout->addLayout(bottomButtonLayout);

    // Layout principal que contiene el widget central de la interfaz
    QVBoxLayout *mainLayoutWidget = new QVBoxLayout(ui->centralwidget);
    mainLayoutWidget->addStretch();
    mainLayoutWidget->addWidget(centralBox, 0, Qt::AlignCenter);
    mainLayoutWidget->addStretch();
    ui->centralwidget->setLayout(mainLayoutWidget);

    // Configuración de los adornos decorativos en las esquinas
    ornamentSize = QSize(300, 299);
    QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");

    // Esquina superior izquierda
    cornerTopLeft = new QLabel(ui->centralwidget);
    QPixmap topLeftPixmap = ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cornerTopLeft->setPixmap(topLeftPixmap);
    cornerTopLeft->setFixedSize(ornamentSize);
    cornerTopLeft->setAttribute(Qt::WA_TransparentForMouseEvents);
    cornerTopLeft->setAttribute(Qt::WA_TranslucentBackground);
    cornerTopLeft->setStyleSheet("background: transparent;");
    cornerTopLeft->raise();

    // Esquina superior derecha (imagen invertida horizontalmente)
    cornerTopRight = new QLabel(ui->centralwidget);
    QTransform transformH;
    transformH.scale(-1, 1);
    QPixmap topRightPixmap = ornamentPixmap.transformed(transformH, Qt::SmoothTransformation)
                                 .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cornerTopRight->setPixmap(topRightPixmap);
    cornerTopRight->setFixedSize(ornamentSize);
    cornerTopRight->setAttribute(Qt::WA_TransparentForMouseEvents);
    cornerTopRight->setAttribute(Qt::WA_TranslucentBackground);
    cornerTopRight->setStyleSheet("background: transparent;");
    cornerTopRight->raise();

    // Esquina inferior izquierda (imagen invertida verticalmente)
    cornerBottomLeft = new QLabel(ui->centralwidget);
    QTransform transformV;
    transformV.scale(1, -1);
    QPixmap bottomLeftPixmap = ornamentPixmap.transformed(transformV, Qt::SmoothTransformation)
                                   .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cornerBottomLeft->setPixmap(bottomLeftPixmap);
    cornerBottomLeft->setFixedSize(ornamentSize);
    cornerBottomLeft->setAttribute(Qt::WA_TransparentForMouseEvents);
    cornerBottomLeft->setAttribute(Qt::WA_TranslucentBackground);
    cornerBottomLeft->setStyleSheet("background: transparent;");
    cornerBottomLeft->raise();

    // Esquina inferior derecha (imagen invertida horizontal y verticalmente)
    cornerBottomRight = new QLabel(ui->centralwidget);
    QTransform transformHV;
    transformHV.scale(-1, -1);
    QPixmap bottomRightPixmap = ornamentPixmap.transformed(transformHV, Qt::SmoothTransformation)
                                    .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cornerBottomRight->setPixmap(bottomRightPixmap);
    cornerBottomRight->setFixedSize(ornamentSize);
    cornerBottomRight->setAttribute(Qt::WA_TransparentForMouseEvents);
    cornerBottomRight->setAttribute(Qt::WA_TranslucentBackground);
    cornerBottomRight->setStyleSheet("background: transparent;");
    cornerBottomRight->raise();

    // Reposicionar los adornos en las esquinas según el tamaño actual del widget central
    repositionOrnaments();
}

/**
 * @brief Destructor de MainWindow.
 *
 * Libera la memoria utilizada por la interfaz de usuario.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Evento de redimensionamiento.
 *
 * Se invoca cuando la ventana cambia de tamaño, lo que permite reajustar la posición
 * de los adornos decorativos.
 *
 * @param event Evento de redimensionamiento.
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    repositionOrnaments();
}

/**
 * @brief Reposiciona los adornos decorativos en las esquinas del widget central.
 *
 * Calcula la posición en función del tamaño actual del widget central para mantener
 * la coherencia visual en la interfaz.
 */
void MainWindow::repositionOrnaments()
{
    if (!ui->centralwidget) return;
    int cw = ui->centralwidget->width();
    int ch = ui->centralwidget->height();

    cornerTopLeft->move(0, 0);
    cornerTopRight->move(cw - cornerTopRight->width(), 0);
    cornerBottomLeft->move(0, ch - cornerBottomLeft->height());
    cornerBottomRight->move(cw - cornerBottomRight->width(), ch - cornerBottomRight->height());
}

/**
 * @brief Abre la ventana de inicio de sesión.
 *
 * Crea una instancia de LoginWindow con la ventana principal como padre,
 * conecta la señal para solicitar el registro y muestra el diálogo de login de forma modal.
 */
void MainWindow::openLoginWindow()
{
    LoginWindow *loginWin = new LoginWindow(this);
    connect(loginWin, &LoginWindow::openRegisterRequested,
            this, &MainWindow::handleOpenRegisterRequested);
    loginWin->exec();
}

/**
 * @brief Abre la ventana de registro.
 *
 * Crea una instancia de RegisterWindow con la ventana principal como padre,
 * conecta la señal para solicitar el inicio de sesión y muestra el diálogo de registro de forma modal.
 */
void MainWindow::openRegisterWindow()
{
    RegisterWindow *regWin = new RegisterWindow(this);
    connect(regWin, &RegisterWindow::openLoginRequested,
            this, &MainWindow::handleOpenLoginRequested);
    regWin->exec();
}

/**
 * @brief Maneja la señal de RegisterWindow para solicitar iniciar sesión.
 *
 * Responde a la señal emitida cuando el usuario, desde la ventana de registro,
 * indica que ya posee una cuenta y desea iniciar sesión.
 */
void MainWindow::handleOpenLoginRequested()
{
    // Se cierra cualquier ventana de registro abierta y se abre la de login.
    openLoginWindow();
}

/**
 * @brief Maneja la señal de LoginWindow para solicitar el registro.
 *
 * Responde a la señal emitida cuando el usuario, desde la ventana de login,
 * indica que no tiene cuenta y desea crear una.
 */
void MainWindow::handleOpenRegisterRequested()
{
    // Se cierra la ventana de login (si no se ha cerrado sola) y se abre la de registro.
    openRegisterWindow();
}
