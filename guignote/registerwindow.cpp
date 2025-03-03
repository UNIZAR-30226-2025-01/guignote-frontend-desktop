#include "registerwindow.h"
#include "loginwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFontDatabase>
#include <QIcon>
#include <QAction>
#include <QToolButton>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>


RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
{
    // Configurar la ventana sin bordes y tamaño fijo
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setFixedSize(540, 600);
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

    // Título "Registrarse"
    QLabel *titleLabel = new QLabel("Crear Cuenta", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Estilo para QLineEdit con mayor padding para evitar recortes
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
    usernameEdit->setPlaceholderText("Usuario");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    // Agregar icono de usuario a la izquierda
    usernameEdit->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

    // Campo para el correo electrónico
    QLineEdit *emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Correo Electrónico");
    emailEdit->setStyleSheet(lineEditStyle);
    emailEdit->setFixedWidth(250);
    emailEdit->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(emailEdit, 0, Qt::AlignCenter);

    // Campo para la contraseña
    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    // Se agrega espacio a la derecha para el ícono de alternar visibilidad
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    passwordEdit->setFixedWidth(250);
    // Agregar icono de candado a la izquierda
    passwordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    // Añadir acción con icono para alternar visibilidad de la contraseña
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    // Variable local para el estado de visibilidad
    bool passwordHidden = true;
    connect(togglePasswordAction, &QAction::triggered, this, [=]() mutable {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        QIcon icon(passwordHidden ? QIcon(":/icons/hide_password.png") : QIcon(":/icons/show_password.png"));
        togglePasswordAction->setIcon(icon);
    });
    // Ajustar el QToolButton del toggle
    QToolButton *toggleButton = passwordEdit->findChild<QToolButton*>();
    if (toggleButton) {
        toggleButton->setIconSize(QSize(30, 30));
        toggleButton->move(toggleButton->x() - 5, toggleButton->y());
    }

    // Campo para confirmar la contraseña
    QLineEdit *confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setPlaceholderText("Confirmar Contraseña");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    confirmPasswordEdit->setFixedWidth(250);
    // Agregar icono de candado a la izquierda
    confirmPasswordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(confirmPasswordEdit, 0, Qt::AlignCenter);

    // Añadir acción para alternar visibilidad en el campo de confirmar contraseña
    QAction *toggleConfirmAction = confirmPasswordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    bool confirmPasswordHidden = true;
    connect(toggleConfirmAction, &QAction::triggered, this, [=]() mutable {
        confirmPasswordHidden = !confirmPasswordHidden;
        confirmPasswordEdit->setEchoMode(confirmPasswordHidden ? QLineEdit::Password : QLineEdit::Normal);
        QIcon icon(confirmPasswordHidden ? QIcon(":/icons/hide_password.png") : QIcon(":/icons/show_password.png"));
        toggleConfirmAction->setIcon(icon);
    });
    QToolButton *toggleConfirmButton = confirmPasswordEdit->findChild<QToolButton*>();
    if (toggleConfirmButton) {
        toggleConfirmButton->setIconSize(QSize(30, 30));
        toggleConfirmButton->move(toggleConfirmButton->x() - 5, toggleConfirmButton->y());
    }

    // Layout extra con un botón estilo enlace para los usuarios que ya tengan cuenta
    QHBoxLayout *extraLayout = new QHBoxLayout();
    QPushButton *loginButtonLink = new QPushButton("¿Ya tienes cuenta? Inicia sesión", this);
    loginButtonLink->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout->addWidget(loginButtonLink);
    mainLayout->addLayout(extraLayout);

    // Conexión para mostrar la ventana de inicio de sesión
    connect(loginButtonLink, &QPushButton::clicked, [=]() {
        LoginWindow *loginWin = new LoginWindow();
        loginWin->move(this->geometry().center() - loginWin->rect().center());
        loginWin->show();
    });
    connect(loginButtonLink, &QPushButton::clicked, this, &RegisterWindow::close);


    // Botón para registrarse
    QPushButton *registerButton = new QPushButton("Crear Cuenta", this);
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
    registerButton->setStyleSheet(buttonStyle);
    registerButton->setFixedSize(200, 50);
    mainLayout->addWidget(registerButton, 0, Qt::AlignCenter);

    // FUNCIÓN PARA CONECTAR EL BOTÓN DE REGISTRO A LA PETICIÓN DJANGO (cortesía de José Manuel)
    connect(registerButton, &QPushButton::clicked, this, [=]() {
        // Recolectar datos de los campos
        QString nombre = usernameEdit->text().trimmed();
        QString correo = emailEdit->text().trimmed();
        QString contrasegna = passwordEdit->text();
        QString confirmContrasegna = confirmPasswordEdit->text();

        // Validar que las contraseñas coincidan
        if (contrasegna != confirmContrasegna) {
            qWarning() << "Las contraseñas no coinciden.";
            return;
        }

        // Crear el objeto JSON con los datos
        QJsonObject json;
        json["nombre"] = nombre;
        json["correo"] = correo;
        json["contrasegna"] = contrasegna;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        // Configurar la URL del endpoint del backend
        QUrl url("http://188.165.76.134:8000/usuarios/crear_usuario/");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        // Crear el gestor de red
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
            if (reply->error() == QNetworkReply::NoError) {
                // Procesar la respuesta (por ejemplo, extraer el token JWT)
                QByteArray responseData = reply->readAll();
                QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
                if (responseDoc.isObject()) {
                    QJsonObject respObj = responseDoc.object();
                    if (respObj.contains("token")) {
                        QString token = respObj["token"].toString();
                        qDebug() << "Token recibido:" << token;
                        // Aquí puedes almacenar el token según lo requieras
                    } else {
                        qWarning() << "Respuesta sin token:" << responseData;
                    }
                }
            } else {
                qWarning() << "Error en la petición:" << reply->errorString();
            }
            reply->deleteLater();
            manager->deleteLater();
        });

        // Enviar la petición POST con el JSON generado
        manager->post(request, data);
    });



    // Botón para volver
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);

    // Conectar el botón "Volver" para cerrar la ventana
    connect(backButton, &QPushButton::clicked, this, &RegisterWindow::close);
}

RegisterWindow::~RegisterWindow() {}
