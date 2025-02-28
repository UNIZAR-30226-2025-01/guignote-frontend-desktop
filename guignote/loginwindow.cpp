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
    // Configurar la ventana sin bordes y con fondo transparente
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    // Aumentamos ligeramente el tamaño para que el título y los campos encajen bien
    setFixedSize(500, 500);

    // Aplicar el estilo del recuadro negro a la ventana
    this->setStyleSheet("background-color: #171718; border-radius: 5px; padding: 20px;");

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

    // Campo para el usuario
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario");
    usernameEdit->setFixedHeight(50);
    QString lineEditStyle = "QLineEdit {"
                            "background-color: #c2c2c3;"
                            "color: #171718;"
                            "border-radius: 15px;"
                            "font-size: 20px;"
                            "padding-left: 10px;"
                            "}";
    usernameEdit->setStyleSheet(lineEditStyle);
    mainLayout->addWidget(usernameEdit);

    // Campo para la contraseña
    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(50);
    // Se aplica un padding a la izquierda y se deja espacio a la derecha para el icono
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");

    // Añadir acción con icono en la parte derecha del campo de contraseña
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon("icons/hide_password.png"), QLineEdit::TrailingPosition);
    // Conectar la acción para alternar la visibilidad de la contraseña
    connect(togglePasswordAction, &QAction::triggered, this, [=]() {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        // Cambiar el icono según el estado
        QIcon icon(passwordHidden ? QIcon("icons/hide_password.png") : QIcon("icons/show_password.png"));
        togglePasswordAction->setIcon(icon);
    });
    mainLayout->addWidget(passwordEdit);

    // Layout horizontal para "He olvidado mi contraseña" y checkbox
    QHBoxLayout *extraLayout = new QHBoxLayout();
    // Etiqueta para "He olvidado mi contraseña"
    QLabel *forgotLabel = new QLabel("He olvidado mi contraseña", this);
    forgotLabel->setStyleSheet("color: #ffffff; text-decoration: underline; font-size: 14px;");
    extraLayout->addWidget(forgotLabel);
    // Checkbox para "Recordar contraseña"
    QCheckBox *rememberCheck = new QCheckBox("Recordar contraseña", this);
    rememberCheck->setStyleSheet("color: #ffffff; font-size: 14px;");
    extraLayout->addWidget(rememberCheck);
    mainLayout->addLayout(extraLayout);

    // Botón para iniciar sesión
    QPushButton *loginButton = new QPushButton("Iniciar Sesión", this);
    QString buttonStyle = "QPushButton {"
                          "background-color: #c2c2c3;"
                          "color: #171718;"
                          "border-radius: 15px;"
                          "font-size: 20px;"
                          "font-weight: bold;"
                          "padding: 12px 25px;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: #0056b3;"
                          "border: 2px solid #e0e0e0;"
                          "}";
    loginButton->setStyleSheet(buttonStyle);
    loginButton->setFixedSize(250, 50);
    mainLayout->addWidget(loginButton, 0, Qt::AlignCenter);

    // Botón para volver
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(100, 40);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);

    // Conectar el botón "Volver" para cerrar la ventana
    connect(backButton, &QPushButton::clicked, this, &LoginWindow::close);
}

LoginWindow::~LoginWindow() {}
