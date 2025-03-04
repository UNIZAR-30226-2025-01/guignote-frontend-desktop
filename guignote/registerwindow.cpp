#include "registerwindow.h"
#include "loginwindow.h"
#include "menuwindow.h"

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
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QDialog(parent), backgroundOverlay(nullptr)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(500, 600);
    setStyleSheet("background-color: #171718; border-radius: 5px; padding: 20px;");

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

    QLabel *titleLabel = new QLabel("Crear Cuenta", this);
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
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    usernameEdit->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

    QLineEdit *emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Correo Electrónico");
    emailEdit->setStyleSheet(lineEditStyle);
    emailEdit->setFixedWidth(250);
    emailEdit->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(emailEdit, 0, Qt::AlignCenter);

    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    passwordEdit->setFixedWidth(250);
    passwordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    bool passwordHidden = true;
    connect(togglePasswordAction, &QAction::triggered, this, [=]() mutable {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        QIcon icon(passwordHidden ? QIcon(":/icons/hide_password.png") : QIcon(":/icons/show_password.png"));
        togglePasswordAction->setIcon(icon);
    });
    QToolButton *toggleButton = passwordEdit->findChild<QToolButton*>();
    if (toggleButton) {
        toggleButton->setIconSize(QSize(30, 30));
        toggleButton->move(toggleButton->x() - 5, toggleButton->y());
    }

    QLineEdit *confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setPlaceholderText("Confirmar Contraseña");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    confirmPasswordEdit->setFixedWidth(250);
    confirmPasswordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(confirmPasswordEdit, 0, Qt::AlignCenter);

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

    QHBoxLayout *extraLayout = new QHBoxLayout();
    QPushButton *loginButtonLink = new QPushButton("¿Ya tienes cuenta? Inicia sesión", this);
    loginButtonLink->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout->addWidget(loginButtonLink);
    mainLayout->addLayout(extraLayout);
    connect(loginButtonLink, &QPushButton::clicked, [=]() {
        LoginWindow *loginWin = new LoginWindow(this);
        loginWin->move(this->geometry().center() - loginWin->rect().center());
        loginWin->show();
    });
    connect(loginButtonLink, &QPushButton::clicked, this, &QDialog::close);

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

    connect(registerButton, &QPushButton::clicked, this, [=]() {
        QString nombre = usernameEdit->text().trimmed();
        QString correo = emailEdit->text().trimmed();
        QString contrasegna = passwordEdit->text();
        QString confirmContrasegna = confirmPasswordEdit->text();

        if (contrasegna != confirmContrasegna) {
            qWarning() << "Las contraseñas no coinciden.";
            return;
        }

        QJsonObject json;
        json["nombre"] = nombre;
        json["correo"] = correo;
        json["contrasegna"] = contrasegna;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        QUrl url("http://188.165.76.134:8000/usuarios/crear_usuario/");
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

        manager->post(request, data);
    });

    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    connect(backButton, &QPushButton::clicked, this, &QDialog::close);
}

RegisterWindow::~RegisterWindow() {}

void RegisterWindow::showEvent(QShowEvent *event)
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

void RegisterWindow::closeEvent(QCloseEvent *event)
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

bool RegisterWindow::eventFilter(QObject *watched, QEvent *event)
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
