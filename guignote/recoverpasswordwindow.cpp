#include "recoverpasswordwindow.h"
#include "loginwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFontDatabase>
#include <QIcon>
#include <QAction>
#include <QDebug>

RecoverPasswordWindow::RecoverPasswordWindow(QWidget *parent)
    : QWidget(parent)
{
    // Configurar la ventana sin bordes y tamaño fijo
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setFixedSize(480, 500);
    setStyleSheet("background-color: #171718; border-radius: 5px; padding: 20px;");

    // Cargar la fuente personalizada
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QFont titleFont;
    if (fontId != -1) {
        titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    } else {
        qWarning() << "No se pudo cargar la fuente personalizada.";
        titleFont = QFont("Arial", 32, QFont::Bold);
    }

    // Layout principal (vertical)
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Título "Recuperar Contraseña"
    QLabel *titleLabel = new QLabel("Recuperar Contraseña", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Texto informativo
    QLabel *infoLabel = new QLabel("Introduce tu correo electrónico para recibir instrucciones para recuperar tu contraseña.", this);
    infoLabel->setStyleSheet("color: #ffffff; font-size: 14px;");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);

    // Estilo para QLineEdit
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Campo para el correo electrónico
    QLineEdit *emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Correo Electrónico");
    emailEdit->setStyleSheet(lineEditStyle);
    emailEdit->setFixedWidth(250);
    emailEdit->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(emailEdit, 0, Qt::AlignCenter);

    // Botón para enviar instrucciones de recuperación
    QPushButton *recoverButton = new QPushButton("Enviar", this);
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   padding: 12px 25px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #9b9b9b;"
        "}";
    recoverButton->setStyleSheet(buttonStyle);
    recoverButton->setFixedSize(200, 50);
    mainLayout->addWidget(recoverButton, 0, Qt::AlignCenter);

    // Layout extra con un botón estilo enlace para volver a la ventana de inicio de sesión
    QHBoxLayout *extraLayout = new QHBoxLayout();
    QPushButton *loginButtonLink = new QPushButton("Volver al inicio de sesión", this);
    loginButtonLink->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout->addWidget(loginButtonLink);
    mainLayout->addLayout(extraLayout);

    // Conexión para mostrar la ventana de inicio de sesión
    connect(loginButtonLink, &QPushButton::clicked, [=]() {
        LoginWindow *loginWin = new LoginWindow();
        loginWin->move(this->geometry().center() - loginWin->rect().center());
        loginWin->show();
    });
    connect(loginButtonLink, &QPushButton::clicked, this, &RecoverPasswordWindow::close);



}
RecoverPasswordWindow::~RecoverPasswordWindow() {}
