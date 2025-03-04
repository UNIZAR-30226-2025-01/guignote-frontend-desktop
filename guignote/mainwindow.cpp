#include "mainwindow.h"
#include "./ui_mainwindow.h"
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
#include "loginwindow.h"
#include "registerwindow.h"  // Incluir el header de la ventana de registro

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    cornerTopLeft(nullptr), cornerTopRight(nullptr),
    cornerBottomLeft(nullptr), cornerBottomRight(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Inicio");

    // setMinimumSize(1090, 600);
    // setMaximumSize(1920, 1080);
    // resize(1090, 600);

    // Crear centralwidget si no existe
    if (!ui->centralwidget) {
        QWidget *central = new QWidget(this);
        setCentralWidget(central);
    }

    // Fondo con gradiente
    this->setStyleSheet("QWidget {"
                        "background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
                        "fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
                        "}");

    // Caja central (con sombra)
    QFrame *centralBox = new QFrame(ui->centralwidget);
    centralBox->setStyleSheet("QFrame {"
                              "background-color: #171718;"
                              "border-radius: 5px;"
                              "padding: 20px;"
                              "}");
    centralBox->setFixedSize(450, 450);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(centralBox);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(4, 4);
    centralBox->setGraphicsEffect(shadow);

    // Fuente personalizada
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    if (fontId == -1) {
        qWarning() << "No se pudo cargar la fuente personalizada.";
    }

    // Título y logo dentro de centralBox
    QLabel *titleLabel = new QLabel("SOTA, CABALLO Y REY", centralBox);
    QFont titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *boxLayout = new QVBoxLayout(centralBox);
    boxLayout->setAlignment(Qt::AlignCenter);

    QLabel *logoLabel = new QLabel(centralBox);
    QPixmap logoPixmap(":/images/app_logo_white.png");
    logoLabel->setScaledContents(false);
    logoLabel->setFixedSize(150, 150);
    QPixmap scaledLogo = logoPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logoLabel->setPixmap(scaledLogo);

    boxLayout->addWidget(titleLabel);
    boxLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    boxLayout->addSpacing(50);

    QPushButton *loginButton = new QPushButton("Iniciar Sesión", centralBox);
    QPushButton *registerButton = new QPushButton("Crear Cuenta", centralBox);
    QString buttonStyle = "QPushButton {"
                          "background-color: #c2c2c3;"
                          "color: #171718;"
                          "border-radius: 15px;"
                          "font-size: 20px;"
                          "font-weight: bold;"
                          "padding: 12px 25px;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: #9b9b9b;"
                          "}";
    loginButton->setStyleSheet(buttonStyle);
    registerButton->setStyleSheet(buttonStyle);
    loginButton->setFixedSize(250, 50);
    registerButton->setFixedSize(250, 50);

    // Conexión para mostrar la ventana de inicio de sesión
    connect(loginButton, &QPushButton::clicked, [=]() {
        // Crear el LoginWindow como hijo de 'centralBox'
        LoginWindow *loginWin = new LoginWindow(centralBox);
        loginWin->setAttribute(Qt::WA_DeleteOnClose);

        // Calcular la posición para centrar el login dentro de 'centralBox'
        QRect centralRect = centralBox->rect();
        int x = centralRect.center().x() - loginWin->width() / 2;
        int y = centralRect.center().y() - loginWin->height() / 2;
        loginWin->move(x, y);
        loginWin->raise();
        loginWin->show();

        // Conectar la señal "volverClicked" para ocultar el login
        connect(loginWin, SIGNAL(volverClicked()), loginWin, SLOT(hide()));
    });

    // Conexión para mostrar la ventana de registro
    connect(registerButton, &QPushButton::clicked, [=]() {
        // Guardar el tamaño original de centralBox antes de cambiarlo
        static QSize originalSize = centralBox->size();

        // Aumentar temporalmente el tamaño de centralBox
        centralBox->setFixedSize(400, 550);

        RegisterWindow *regWin = new RegisterWindow(centralBox);
        regWin->setAttribute(Qt::WA_DeleteOnClose);

        // Centrar RegisterWindow dentro de centralBox
        QRect centralRect = centralBox->rect();
        int x = centralRect.center().x() - regWin->width() / 2;
        int y = centralRect.center().y() - regWin->height() / 2;
        regWin->move(x, y);

        regWin->raise();
        regWin->show();

        // Restaurar el tamaño de centralBox cuando se oculte RegisterWindow
        connect(regWin, &RegisterWindow::volverClicked, [=]() {
            centralBox->setFixedSize(originalSize);
            regWin->hide();
        });
    });

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setSpacing(30);
    buttonLayout->addWidget(loginButton, 0, Qt::AlignCenter);
    buttonLayout->addWidget(registerButton, 0, Qt::AlignCenter);
    boxLayout->addLayout(buttonLayout);
    centralBox->setLayout(boxLayout);

    // Layout principal para centrar la caja negra
    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->addStretch();
    mainLayout->addWidget(centralBox, 0, Qt::AlignCenter);

    // --- Botón de salida con icono ---
    QPushButton *exitIconButton = new QPushButton(ui->centralwidget);

    // Cargar el pixmap original de la puerta
    QPixmap doorPixmap(":/icons/door.png");
    // Crear el pixmap oscurecido
    QPixmap darkenedDoor = doorPixmap;
    {
        QPainter painter(&darkenedDoor);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(darkenedDoor.rect(), QColor(120, 120, 120, 100));
        painter.end();
    }

    // Asignar el icono normal inicialmente
    exitIconButton->setIcon(QIcon(doorPixmap));
    exitIconButton->setIconSize(QSize(50,50));
    exitIconButton->setFlat(true);
    exitIconButton->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QPushButton:pressed {"
        "   background-color: transparent;"
        "}"
        );
    exitIconButton->setCursor(Qt::PointingHandCursor);

    // Al presionar el botón se oscurece la imagen (aunque no se restablece al soltar)
    connect(exitIconButton, &QPushButton::pressed, [=]() {
        exitIconButton->setIcon(QIcon(darkenedDoor));
    });

    // Al hacer click, antes de mostrar el warning, se mantiene la imagen oscura
    connect(exitIconButton, &QPushButton::clicked, [=]() {
        // Crear el diálogo de confirmación
        QDialog *confirmDialog = new QDialog(this);
        confirmDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        confirmDialog->setModal(true);
        confirmDialog->setFixedSize(300,150);
        confirmDialog->setStyleSheet("QDialog {"
                                     "background-color: #171718;"
                                     "border-radius: 5px;"
                                     "padding: 20px;"
                                     "}");

        QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(confirmDialog);
        dialogShadow->setBlurRadius(10);
        dialogShadow->setColor(QColor(0, 0, 0, 80));
        dialogShadow->setOffset(4, 4);
        confirmDialog->setGraphicsEffect(dialogShadow);

        QVBoxLayout *dialogLayout = new QVBoxLayout(confirmDialog);
        QLabel *confirmLabel = new QLabel("¿Está seguro que desea salir?", confirmDialog);
        confirmLabel->setStyleSheet("QFrame {"
                                    "background-color: #171718;"
                                    "border-radius: 5px;"
                                    "}");
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

        // Al cerrarse el diálogo se restaura el icono original
        connect(confirmDialog, &QDialog::finished, [=](int) {
            exitIconButton->setIcon(QIcon(doorPixmap));
        });

        // Centrar el diálogo en la ventana principal
        confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        confirmDialog->show();
    });

    boxLayout->addWidget(exitIconButton, 0, Qt::AlignRight | Qt::AlignBottom);
    mainLayout->addStretch();
    ui->centralwidget->setLayout(mainLayout);

    // ---- Decoraciones en las esquinas (fuera de centralBox) ----
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

    // Esquina superior derecha (volteada horizontalmente)
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

    // Esquina inferior izquierda (volteada verticalmente)
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

    // Esquina inferior derecha (volteada horizontal y verticalmente)
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

    // Posicionar decoraciones en las esquinas
    repositionOrnaments();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    repositionOrnaments();
}

void MainWindow::repositionOrnaments() {
    if (ui->centralwidget) {
        int cw = ui->centralwidget->width();
        int ch = ui->centralwidget->height();
        cornerTopLeft->move(0, 0);
        cornerTopRight->move(cw - cornerTopRight->width(), 0);
        cornerBottomLeft->move(0, ch - cornerBottomLeft->height());
        cornerBottomRight->move(cw - cornerBottomRight->width(), ch - cornerBottomRight->height());
    }
}

MainWindow::~MainWindow()
{
    delete ui;  // Solo eliminar si `ui` fue inicializado
}
