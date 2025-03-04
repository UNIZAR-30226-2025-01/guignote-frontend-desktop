#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Otras ventanas
#include "loginwindow.h"
#include "registerwindow.h"
#include "settingswindow.h" // Si la usas
#include "icon.h"

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , cornerTopLeft(nullptr)
    , cornerTopRight(nullptr)
    , cornerBottomLeft(nullptr)
    , cornerBottomRight(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Inicio");

    setMinimumSize(1090, 600);
    setMaximumSize(1920, 1080);
    resize(1090, 600);

    // Asegurar widget central
    if (!ui->centralwidget) {
        QWidget *central = new QWidget(this);
        setCentralWidget(central);
    }

    // Fondo de la ventana
    this->setStyleSheet(
        "QWidget {"
        "  background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
        "  fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
        "}"
        );

    // Caja central
    QFrame *centralBox = new QFrame(ui->centralwidget);
    centralBox->setStyleSheet(
        "QFrame {"
        "  background-color: #171718;"
        "  border-radius: 5px;"
        "  padding: 20px;"
        "}"
        );
    centralBox->setFixedSize(450, 450);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(centralBox);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(4, 4);
    centralBox->setGraphicsEffect(shadow);

    // Fuente
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    if (fontId == -1) {
        qWarning() << "No se pudo cargar la fuente personalizada.";
    }

    // Layout interno de centralBox
    QVBoxLayout *boxLayout = new QVBoxLayout(centralBox);
    boxLayout->setAlignment(Qt::AlignCenter);

    // Título
    QLabel *titleLabel = new QLabel("SOTA, CABALLO Y REY", centralBox);
    QFont titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).value(0), 32);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    boxLayout->addWidget(titleLabel);

    // Logo
    QLabel *logoLabel = new QLabel(centralBox);
    QPixmap logoPixmap(":/images/app_logo_white.png");
    logoLabel->setScaledContents(false);
    logoLabel->setFixedSize(150, 150);
    QPixmap scaledLogo = logoPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logoLabel->setPixmap(scaledLogo);
    boxLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    boxLayout->addSpacing(50);

    // Botones de login y registro
    QPushButton *loginButton = new QPushButton("Iniciar Sesión", centralBox);
    QPushButton *registerButton = new QPushButton("Crear Cuenta", centralBox);

    QString buttonStyle =
        "QPushButton {"
        "  background-color: #c2c2c3;"
        "  color: #171718;"
        "  border-radius: 15px;"
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

    // En lugar de abrir la ventana desde aquí, llamamos slots que la crean
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::openLoginWindow);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::openRegisterWindow);

    // Layout para esos dos botones
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
        SettingsWindow *settingsWin = new SettingsWindow(this);
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

    // Layout principal de MainWindow
    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->addStretch();
    mainLayout->addWidget(centralBox, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    ui->centralwidget->setLayout(mainLayout);

    // Adornos de esquina
    ornamentSize = QSize(300, 299);
    QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");

    // Esquina sup izq
    cornerTopLeft = new QLabel(ui->centralwidget);
    QPixmap topLeftPixmap = ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cornerTopLeft->setPixmap(topLeftPixmap);
    cornerTopLeft->setFixedSize(ornamentSize);
    cornerTopLeft->setAttribute(Qt::WA_TransparentForMouseEvents);
    cornerTopLeft->setAttribute(Qt::WA_TranslucentBackground);
    cornerTopLeft->setStyleSheet("background: transparent;");
    cornerTopLeft->raise();

    // Esquina sup der (invertir X)
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

    // Esquina inf izq (invertir Y)
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

    // Esquina inf der (invertir X e Y)
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

    repositionOrnaments();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    repositionOrnaments();
}

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

// Abre la ventana de Login con this como padre
void MainWindow::openLoginWindow()
{
    LoginWindow *loginWin = new LoginWindow(this);

    // Conectar la señal “openRegisterRequested” de la ventana de login
    connect(loginWin, &LoginWindow::openRegisterRequested,
            this, &MainWindow::handleOpenRegisterRequested);

    loginWin->exec();
}

// Abre la ventana de Registro con this como padre
void MainWindow::openRegisterWindow()
{
    RegisterWindow *regWin = new RegisterWindow(this);

    // Conectar la señal “openLoginRequested” de la ventana de registro
    connect(regWin, &RegisterWindow::openLoginRequested,
            this, &MainWindow::handleOpenLoginRequested);

    regWin->exec();
}

/**
 * @brief Respuesta a la señal de RegisterWindow que indica “¿Ya tienes cuenta? Inicia sesión”.
 */
void MainWindow::handleOpenLoginRequested()
{
    // Cerramos cualquier RegisterWindow que esté abierta
    // (La propia RegisterWindow ya se cierra sola al emitir la señal, pero por si acaso.)
    // Después abrimos la de Login:
    openLoginWindow();
}

/**
 * @brief Respuesta a la señal de LoginWindow que indica “¿No tienes cuenta? Crea una”.
 */
void MainWindow::handleOpenRegisterRequested()
{
    // Cerramos el LoginWindow (si no se ha cerrado solo) y abrimos RegisterWindow
    openRegisterWindow();
}
