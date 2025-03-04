#include "loginwindow.h"
#include "menuwindow.h"
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
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent),
    backgroundOverlay(nullptr)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(480, 500);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QFont titleFont;
    if (fontId != -1) {
        titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    } else {
        qWarning() << "No se pudo cargar la fuente personalizada.";
        titleFont = QFont("Arial", 32, QFont::Bold);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("Iniciar Sesión", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Usuario/Correo
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario o Correo");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    usernameEdit->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

    // Contraseña
    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    passwordEdit->setFixedWidth(250);
    passwordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    // Mostrar/ocultar contraseña
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    bool passwordHidden = true;
    connect(togglePasswordAction, &QAction::triggered, this, [=]() mutable {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        togglePasswordAction->setIcon(QIcon(passwordHidden ? ":/icons/hide_password.png" : ":/icons/show_password.png"));
    });

    // Layout horizontal con “¿Olvidaste tu contraseña?” y el checkbox de “Recordar”
    QHBoxLayout *extraLayout = new QHBoxLayout();
    QPushButton *forgotPasswordButton = new QPushButton("¿Has olvidado tu contraseña?", this);
    forgotPasswordButton->setStyleSheet(
        "QPushButton {"
        "  color: #ffffff;"
        "  text-decoration: underline;"
        "  font-size: 14px;"
        "  background: transparent;"
        "  border: none;"
        "}"
        );
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

    // Recuperar contraseña
    connect(forgotPasswordButton, &QPushButton::clicked, this, [=]() {
        RecoverPasswordWindow *recoverWin = new RecoverPasswordWindow(this);
        recoverWin->move(this->geometry().center() - recoverWin->rect().center());
        recoverWin->show();
    });

    // Botón “Iniciar Sesión”
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

    // Lógica para iniciar sesión (petición POST)
    connect(loginButton, &QPushButton::clicked, [=]() {
        QString userOrEmail = usernameEdit->text().trimmed();
        QString contrasegna = passwordEdit->text();

        if (userOrEmail.isEmpty() || contrasegna.isEmpty()) {
            qWarning() << "Faltan campos";
            return;
        }

        QJsonObject json;
        if (userOrEmail.contains('@')) {
            json["correo"] = userOrEmail;
        } else {
            json["nombre"] = userOrEmail;
        }
        json["contrasegna"] = contrasegna;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        QUrl url("http://188.165.76.134:8000/usuarios/iniciar_sesion/");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
                if (responseDoc.isObject()) {
                    QJsonObject respObj = responseDoc.object();
                    if (respObj.contains("token")) {
                        QString token = respObj["token"].toString();
                        qDebug() << "Token recibido:" << token;
                        MenuWindow *menuWin = new MenuWindow(this);
                        menuWin->move(this->geometry().center() - menuWin->rect().center());
                        menuWin->show();
                        this->close();
                    } else if (respObj.contains("error")) {
                        qWarning() << "Error:" << respObj["error"].toString();
                    } else {
                        qWarning() << "Respuesta inesperada:" << responseData;
                    }
                }
            } else {
                qWarning() << "Error en la petición:" << reply->errorString();
            }
            reply->deleteLater();
            manager->deleteLater();
        });

        manager->post(request, data);
    });

    // Botón para volver
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    connect(backButton, &QPushButton::clicked, this, &QDialog::close);

    // Por último, el texto “¿No tienes cuenta? Crea una”
    QHBoxLayout *registerLinkLayout = new QHBoxLayout();
    QPushButton *regButtonLink = new QPushButton("¿Aún no tienes cuenta? Crea una", this);
    regButtonLink->setStyleSheet(
        "QPushButton {"
        "  color: #ffffff;"
        "  text-decoration: underline;"
        "  font-size: 14px;"
        "  background: transparent;"
        "  border: none;"
        "}"
        );
    registerLinkLayout->addWidget(regButtonLink);
    mainLayout->addLayout(registerLinkLayout);

    // Aquí emitimos la señal openRegisterRequested()
    connect(regButtonLink, &QPushButton::clicked, [=]() {
        this->close(); // cerramos LoginWindow
        emit openRegisterRequested();
    });
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (parentWidget()) {
        backgroundOverlay = new QWidget(parentWidget());
        backgroundOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 128);");
        backgroundOverlay->setGeometry(parentWidget()->rect());
        backgroundOverlay->show();
        backgroundOverlay->raise();
        parentWidget()->installEventFilter(this);
        this->move(parentWidget()->geometry().center() - this->rect().center());
    }
}

void LoginWindow::closeEvent(QCloseEvent *event)
{
    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    if (backgroundOverlay) {
        backgroundOverlay->deleteLater();
        backgroundOverlay = nullptr;
    }
    QDialog::closeEvent(event);
}

bool LoginWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        if (parentWidget()) {
            this->move(parentWidget()->geometry().center() - this->rect().center());
            if (backgroundOverlay) {
                backgroundOverlay->setGeometry(parentWidget()->rect());
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}
