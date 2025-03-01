#include "loginwindow.h"
#include "menuwindow.h"
#include "registerwindow.h"
#include "recoverpasswordwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QAction>
#include <QDebug>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent), passwordHidden(true)
{
    // Configurar la ventana sin bordes
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

    // Título "Iniciar Sesión"
    QLabel *titleLabel = new QLabel("Iniciar Sesión", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Estilo para QLineEdit
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Campo para el usuario
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario o Correo");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    usernameEdit->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

    // Campo para la contraseña
    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    passwordEdit->setFixedWidth(250);
    passwordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    // Acción para alternar visibilidad de la contraseña
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    bool passwordHidden = true;
    connect(togglePasswordAction, &QAction::triggered, this, [=]() mutable {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        QIcon icon(passwordHidden ? QIcon(":/icons/hide_password.png") : QIcon(":/icons/show_password.png"));
        togglePasswordAction->setIcon(icon);
    });

    // Layout extra para "olvido de contraseña" y checkbox
    QHBoxLayout *extraLayout = new QHBoxLayout();

    // Se utiliza un QPushButton para "¿Has olvidado tu contraseña?" (enlace clickable)
    QPushButton *forgotPasswordButton = new QPushButton("¿Has olvidado tu contraseña?", this);
    forgotPasswordButton->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout->addWidget(forgotPasswordButton);

    QCheckBox *rememberCheck = new QCheckBox("Recordar contraseña", this);
    QString checkBoxStyle = R"(
        QCheckBox {
            color: #ffffff;
            font-size: 14px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        QCheckBox::indicator:unchecked {
            background-color: #c2c2c3;
            border: 1px solid #545454;
            image: none;
        }
        QCheckBox::indicator:checked {
            background-color: #c2c2c3;
            border: 1px solid #545454;
            image: url(":/icons/cross.png");
        }
    )";
    rememberCheck->setStyleSheet(checkBoxStyle);
    extraLayout->addWidget(rememberCheck);
    mainLayout->addLayout(extraLayout);

    // Conectar el botón "olvidé la contraseña" a la pantalla de recuperación
    connect(forgotPasswordButton, &QPushButton::clicked, [=]() {
        RecoverPasswordWindow *recoverWin = new RecoverPasswordWindow();
        recoverWin->move(this->geometry().center() - recoverWin->rect().center());
        recoverWin->show();
    });
    connect(forgotPasswordButton, &QPushButton::clicked, this, &LoginWindow::close);

    // Botón para iniciar sesión
    QPushButton *loginButton = new QPushButton("Iniciar Sesión", this);
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   padding: 12px 25px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #9b9b9b;"
        "}";
    loginButton->setStyleSheet(buttonStyle);
    loginButton->setFixedSize(200, 50);
    mainLayout->addWidget(loginButton, 0, Qt::AlignCenter);

    // Conexión para el botón de "Iniciar Sesión" con el menú principal
    connect(loginButton, &QPushButton::clicked, [=]() {
        // TODO: Conectar con el backend para validar credenciales

        // Conexión con el menú principal
        MenuWindow *menuWin = new MenuWindow();
        menuWin->move(this->geometry().center() - menuWin->rect().center());
        menuWin->show();
        this->close();
    });

    // Botón para volver
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    connect(backButton, &QPushButton::clicked, this, &LoginWindow::close);

    // Layout extra con botón para ir a la ventana de registro
    QHBoxLayout *extraLayout1 = new QHBoxLayout();
    QPushButton *regButtonLink = new QPushButton("¿Aún no tienes cuenta? Crea una nueva", this);
    regButtonLink->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout1->addWidget(regButtonLink);
    mainLayout->addLayout(extraLayout1);
    connect(regButtonLink, &QPushButton::clicked, [=]() {
        RegisterWindow *regWin = new RegisterWindow();
        regWin->move(this->geometry().center() - regWin->rect().center());
        regWin->show();
    });
    connect(regButtonLink, &QPushButton::clicked, this, &LoginWindow::close);
}

LoginWindow::~LoginWindow() {}
