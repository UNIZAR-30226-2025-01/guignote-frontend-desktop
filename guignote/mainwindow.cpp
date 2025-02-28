#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QTransform>
#include "loginwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    cornerTopLeft(nullptr), cornerTopRight(nullptr),
    cornerBottomLeft(nullptr), cornerBottomRight(nullptr)
{
    ui->setupUi(this);

    setMinimumSize(1090, 600);
    setMaximumSize(1920, 1080);
    resize(1090, 600);

    // Si no existe, creamos el centralwidget
    if (!ui->centralwidget) {
        QWidget *central = new QWidget(this);
        setCentralWidget(central);
    }

    // Fondo con gradiente
    this->setStyleSheet("QWidget {"
                        "background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
                        "fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
                        "}");

    // Caja central (con shadow)
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
    QPushButton *registerButton = new QPushButton("Registrarse", centralBox);
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

    connect(loginButton, &QPushButton::clicked, [=]() {
        LoginWindow *loginWin = new LoginWindow();
        loginWin->move(this->geometry().center() - loginWin->rect().center());
        loginWin->show();
    });


    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->addWidget(loginButton, 0, Qt::AlignCenter);
    buttonLayout->addWidget(registerButton, 0, Qt::AlignCenter);
    buttonLayout->setSpacing(20);
    boxLayout->addLayout(buttonLayout);
    centralBox->setLayout(boxLayout);

    // Layout principal para centrar la caja negra
    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->addStretch();
    mainLayout->addWidget(centralBox, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    ui->centralwidget->setLayout(mainLayout);

    // ---- Decoraciones en las esquinas (fuera de centralBox) ----
    ornamentSize = QSize(300, 299);
    QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");

    // Esquina superior izquierda (original)
    cornerTopLeft = new QLabel(ui->centralwidget);
    QPixmap topLeftPixmap = ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cornerTopLeft->setPixmap(topLeftPixmap);
    cornerTopLeft->setFixedSize(ornamentSize);
    cornerTopLeft->setAttribute(Qt::WA_TransparentForMouseEvents);
    cornerTopLeft->setAttribute(Qt::WA_TranslucentBackground);
    cornerTopLeft->setStyleSheet("background: transparent;");
    cornerTopLeft->raise();

    // Esquina superior derecha (voltear horizontalmente)
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

    // Esquina inferior izquierda (voltear verticalmente)
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

    // Esquina inferior derecha (voltear horizontal y verticalmente)
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

    // Posicionar las decoraciones (se actualizará en resizeEvent)
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
    delete ui;
}
