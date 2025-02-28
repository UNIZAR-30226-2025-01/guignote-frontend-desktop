#include "loginwindow.h"

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
    // Ajustar un tamaño fijo (puedes modificarlo según tu preferencia)
    setFixedSize(480, 500);


    // Estilo general de la ventana
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

    // Estilo para QLineEdit con más padding vertical y tamaño de fuente menor para evitar recortes
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"     // Se reduce un poco la fuente
        "   padding: 8px 10px;"   // Aumenta padding para evitar cortes
        "}";

    // Campo para el usuario
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

    // Campo para la contraseña
    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    // Agregamos espacio a la derecha para el icono
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    passwordEdit->setFixedWidth(250);
    mainLayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    // Añadir acción con icono en la parte derecha del campo de contraseña
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon("icons/hide_password.png"), QLineEdit::TrailingPosition);
    connect(togglePasswordAction, &QAction::triggered, this, [=]() {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        // Cambiar el icono según el estado
        QIcon icon(passwordHidden ? QIcon("icons/hide_password.png") : QIcon("icons/show_password.png"));
        togglePasswordAction->setIcon(icon);
    });

    QHBoxLayout *extraLayout = new QHBoxLayout();

    QLabel *forgotLabel = new QLabel("he olvidado mi contraseña", this);
    forgotLabel->setStyleSheet("color: #ffffff; text-decoration: underline; font-size: 14px;");
    extraLayout->addWidget(forgotLabel);

    // Checkbox para "Recordar contraseña"
    QCheckBox *rememberCheck = new QCheckBox("Recordar contraseña", this);
    QString checkBoxStyle = R"(
    QCheckBox {
        color: #ffffff;             /* Color del texto */
        font-size: 14px;
    }
    QCheckBox::indicator {
        width: 16px;               /* Ajusta el tamaño del indicador */
        height: 16px;
    }
    QCheckBox::indicator:unchecked {
        background-color: #c2c2c3; /* Fondo cuando está sin marcar */
        border: 1px solid #545454; /* Borde */
        image: none;
    }
    QCheckBox::indicator:checked {
        background-color: #545454; /* Fondo cuando está marcado */
        border: 1px solid #e0e0e0;
        image: url("icons/cross.png"); /* Aquí se muestra la imagen de la cruz */
    }
)";

    rememberCheck->setStyleSheet(checkBoxStyle);
    extraLayout->addWidget(rememberCheck);
    mainLayout->addLayout(extraLayout);

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

    // Botón para volver
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);

    // Conectar el botón "Volver" para cerrar la ventana
    connect(backButton, &QPushButton::clicked, this, &LoginWindow::close);
}

LoginWindow::~LoginWindow() {}
