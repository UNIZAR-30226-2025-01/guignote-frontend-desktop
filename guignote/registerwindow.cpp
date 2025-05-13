/**
 * @file registerwindow.cpp
 * @brief Implementación de la clase RegisterWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Esta clase representa la ventana de registro de la aplicación. Permite a los usuarios
 * crear una nueva cuenta proporcionando sus credenciales, y contiene la lógica para enviar
 * los datos al servidor mediante una petición HTTP POST. Además, ofrece la opción de cambiar
 * a la ventana de inicio de sesión a través de una señal.
 */

#include "registerwindow.h"
#include "mainwindow.h"
#include "menuwindow.h" // Si se va a abrir MenuWindow tras el registro
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFontDatabase>
#include <QToolButton>
#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

/**
 * @brief Constructor de RegisterWindow.
 * @param parent Puntero al widget padre, puede ser nullptr.
 *
 * Configura el diálogo (modal, sin bordes), carga la fuente personalizada,
 * crea y organiza los widgets: campos de usuario, correo, contraseña,
 * confirmación, enlaces y botones, y prepara el overlay para el fondo.
 */
RegisterWindow::RegisterWindow(QWidget *parent)
    : QDialog(parent),
    backgroundOverlay(nullptr)
{
    // Configuración básica de la ventana: modal, sin bordes y con fondo personalizado.
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(500, 600);
    setStyleSheet("background-color: #171718; border-radius: 5px; padding: 20px;");

    // Cargar la fuente personalizada para el título; si falla, se utiliza una fuente por defecto.
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QFont titleFont;
    if (fontId != -1) {
        titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    } else {
        qWarning() << "No se pudo cargar la fuente personalizada.";
        titleFont = QFont("Arial", 32, QFont::Bold);
    }

    // Creación del layout principal de la ventana.
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Título de la ventana de registro.
    QLabel *titleLabel = new QLabel("Crear Cuenta", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Estilo común para los campos de entrada (QLineEdit).
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Campo para ingresar el nombre de usuario.
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    usernameEdit->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

    // Campo para ingresar el correo electrónico.
    QLineEdit *emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Correo Electrónico");
    emailEdit->setStyleSheet(lineEditStyle);
    emailEdit->setFixedWidth(250);
    emailEdit->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(emailEdit, 0, Qt::AlignCenter);

    // Campo para ingresar la contraseña.
    QLineEdit *passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Contraseña");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    passwordEdit->setFixedWidth(250);
    passwordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    // Acción para alternar la visibilidad de la contraseña.
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    bool passwordHidden = true;
    connect(togglePasswordAction, &QAction::triggered, this, [=]() mutable {
        passwordHidden = !passwordHidden;
        passwordEdit->setEchoMode(passwordHidden ? QLineEdit::Password : QLineEdit::Normal);
        togglePasswordAction->setIcon(QIcon(passwordHidden ? ":/icons/hide_password.png" : ":/icons/show_password.png"));
    });

    // Campo para confirmar la contraseña.
    QLineEdit *confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setPlaceholderText("Confirmar Contraseña");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setStyleSheet(lineEditStyle + "padding-right: 40px;");
    confirmPasswordEdit->setFixedWidth(250);
    confirmPasswordEdit->addAction(QIcon(":/icons/padlock.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(confirmPasswordEdit, 0, Qt::AlignCenter);

    // Acción para alternar la visibilidad en el campo de confirmación.
    QAction *toggleConfirmAction = confirmPasswordEdit->addAction(QIcon(":/icons/hide_password.png"), QLineEdit::TrailingPosition);
    bool confirmPasswordHidden = true;
    connect(toggleConfirmAction, &QAction::triggered, this, [=]() mutable {
        confirmPasswordHidden = !confirmPasswordHidden;
        confirmPasswordEdit->setEchoMode(confirmPasswordHidden ? QLineEdit::Password : QLineEdit::Normal);
        toggleConfirmAction->setIcon(QIcon(confirmPasswordHidden ? ":/icons/hide_password.png" : ":/icons/show_password.png"));
    });

    // Layout para el enlace "¿Ya tienes cuenta? Inicia sesión"
    QHBoxLayout *loginLinkLayout = new QHBoxLayout();
    QPushButton *loginButtonLink = new QPushButton("¿Ya tienes cuenta? Inicia sesión", this);
    loginButtonLink->setStyleSheet(
        "QPushButton {"
        "  color: #ffffff;"
        "  text-decoration: underline;"
        "  font-size: 14px;"
        "  background: transparent;"
        "  border: none;"
        "}"
        );
    loginLinkLayout->addWidget(loginButtonLink);
    mainLayout->addLayout(loginLinkLayout);

    // Conexión para cerrar la ventana de registro y emitir la señal para abrir la ventana de login.
    connect(loginButtonLink, &QPushButton::clicked, this, [=]() {
        this->close();            // Cerramos la ventana de registro.
        emit openLoginRequested(); // Se notifica a MainWindow para abrir LoginWindow.
    });

    // Botón para enviar los datos de registro y crear la cuenta.
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


    // Conexión para el botón "Crear Cuenta": valida los campos, envía una petición POST y maneja la respuesta.
    connect(registerButton, &QPushButton::clicked, this, [=]() {
        QString nombre = usernameEdit->text().trimmed();
        QString correo = emailEdit->text().trimmed();
        QString contrasegna = passwordEdit->text();
        QString confirmContrasegna = confirmPasswordEdit->text();

        // Verificar que ambas contraseñas coincidan.
        if (contrasegna != confirmContrasegna) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Error de autenticación");
            msgBox.setText("Las contraseñas no coinciden.");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStyleSheet("QLabel { color: #ffffff; font-size: 14px; } "
                                 "QPushButton { background-color: #c2c2c3; border: none; border-radius: 5px; padding: 5px; } "
                                 "QMessageBox { background-color: #171718; }");
            msgBox.exec();
            return;
            return;
        }

        // Preparar el objeto JSON con los datos de registro.
        QJsonObject json;
        json["nombre"] = nombre;
        json["correo"] = correo;
        json["contrasegna"] = contrasegna;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        // Configurar la petición HTTP POST a la URL de creación de usuario.
        QUrl url("http://188.165.76.134:8000/usuarios/crear_usuario/");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            // Procesar la respuesta del servidor.
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
                if (responseDoc.isObject()) {
                    QJsonObject respObj = responseDoc.object();
                    // Si se recibe un token, se asume que el registro fue exitoso y se abre MainWindow.
                    if (respObj.contains("token")) {
                        QString token = respObj["token"].toString();
                        int userId = respObj.value("user_id").toInt();
                        qDebug() << "Token recibido:" << token;
                        // Almacenamos el token de forma persistente
                        QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
                        // Almacenamos el token de forma persistente
                        settings.setValue("auth/token", token);

                        // —————————————— Inserta aquí ——————————————
                        // 1) Obtener el user_id a partir del nombre de usuario
                        QNetworkAccessManager *mgrId = new QNetworkAccessManager(this);
                        QUrl urlId(QStringLiteral(
                                       "http://188.165.76.134:8000/usuarios/usuarios/id/%1/")
                                       .arg(nombre));
                        QNetworkRequest reqId(urlId);
                        auto *replyId = mgrId->get(reqId);
                        connect(replyId, &QNetworkReply::finished, this, [this, replyId, mgrId]() {
                            replyId->deleteLater();
                            if (replyId->error() != QNetworkReply::NoError) {
                                qWarning() << "Error al obtener user_id:" << replyId->errorString();
                                mgrId->deleteLater();
                                return;
                            }
                            QJsonDocument doc = QJsonDocument::fromJson(replyId->readAll());
                            int userId = doc.object().value("user_id").toInt(-1);
                            mgrId->deleteLater();
                            if (userId < 0) {
                                qWarning() << "user_id no válido";
                                return;
                            }

                            // 2) Desbloquear siempre la skin 1 tras el registro
                            QNetworkAccessManager *mgrUnlock = new QNetworkAccessManager(this);
                            QUrl urlUnlock(QStringLiteral(
                                               "http://188.165.76.134:8000/usuarios/unlock_skin/%1/")
                                               .arg(userId));
                            QNetworkRequest reqUnlock(urlUnlock);
                            reqUnlock.setHeader(QNetworkRequest::ContentTypeHeader,
                                                "application/json");
                            QJsonObject body;
                            body["skin_id"] = 1;
                            auto *replyUnlock = mgrUnlock->post(reqUnlock, QJsonDocument(body).toJson());
                            connect(replyUnlock, &QNetworkReply::finished, this, [replyUnlock, mgrUnlock]() {
                                if (replyUnlock->error() != QNetworkReply::NoError)
                                    qWarning() << "unlock_skin(1) failed:" << replyUnlock->errorString();
                                else
                                    qDebug() << "unlock_skin(1) success";
                                replyUnlock->deleteLater();
                                mgrUnlock->deleteLater();
                            });

                            QJsonObject body2; body2["skin_id"] = 2;
                            QNetworkReply *replyUnlock2 = mgrUnlock->post(reqUnlock, QJsonDocument(body2).toJson());
                            connect(replyUnlock2, &QNetworkReply::finished, this, [replyUnlock2, mgrUnlock]() {
                                if (replyUnlock2->error() != QNetworkReply::NoError)
                                    qWarning() << "unlock_skin(2) failed:" << replyUnlock2->errorString();
                                else
                                    qDebug()   << "unlock_skin(2) success";
                                replyUnlock2->deleteLater();
                                mgrUnlock->deleteLater();
                            });

                            // ➕ Tercera petición para skin 3
                            QJsonObject body3; body3["skin_id"] = 3;
                            QNetworkRequest reqUnlock3 = reqUnlock; // mismo URL
                            QNetworkReply *replyUnlock3 = mgrUnlock->post(reqUnlock3, QJsonDocument(body3).toJson());
                            QObject::connect(replyUnlock3, &QNetworkReply::finished, this, [replyUnlock3, mgrUnlock]() {
                                if (replyUnlock3->error() != QNetworkReply::NoError)
                                    qWarning() << "unlock_skin(3) failed:" << replyUnlock3->errorString();
                                else
                                    qDebug() << "unlock_skin(3) success";
                                replyUnlock3->deleteLater();
                                mgrUnlock->deleteLater();
                            });
                        });
                        // ————————————————————————————————————————————————

                        // Haz esto:
                        // 1) Cierra sólo la ventana de registro
                        this->close();
                        // 2) Emite la señal que ya tienes conectada en MainWindow para abrir el login
                        emit openLoginRequested();
                    } else {
                        qWarning() << "Respuesta sin token:" << responseData;
                    }
                }
            } else if (statusCode == 400) {
                // Mostrar mensaje de error estético: usuario no encontrado.
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Error de autenticación");
                msgBox.setText("Correo o nombre de usuario ya en uso o campos sin completar");
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setStyleSheet("QLabel { color: #ffffff; font-size: 14px; } "
                                     "QPushButton { background-color: #c2c2c3; border: none; border-radius: 5px; padding: 5px; } "
                                     "QMessageBox { background-color: #171718; }");
                msgBox.exec();
            } else {
                qWarning() << "Error en la petición:" << reply->errorString();
            }
            reply->deleteLater();
            manager->deleteLater();
        });

        // Enviar la petición POST con los datos de registro.
        manager->post(request, data);
    });

    // Botón para volver y cerrar la ventana de registro.
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    connect(backButton, &QPushButton::clicked, this, &QDialog::close);
}

/**
 * @brief Destructor de RegisterWindow.
 *
 * Libera los recursos utilizados por el diálogo. Los widgets hijos son
 * eliminados automáticamente por Qt.
 */
RegisterWindow::~RegisterWindow()
{
    // Liberación de recursos (se gestiona de forma automática en Qt para los widgets hijos).
}

/**
 * @brief Evento que se ejecuta al mostrar la ventana.
 *
 * Crea un overlay semitransparente sobre el widget padre para resaltar la ventana de registro,
 * instala un filtro de eventos en el widget padre y centra la ventana en el mismo.
 *
 * @param event Evento de tipo QShowEvent.
 */
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

/**
 * @brief Evento que se ejecuta al cerrar la ventana.
 *
 * Elimina el overlay y remueve el filtro de eventos instalado en el widget padre.
 *
 * @param event Evento de tipo QCloseEvent.
 */
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

/**
 * @brief Filtra eventos del widget padre.
 *
 * Permite ajustar la posición de la ventana de registro y del overlay en función de los cambios
 * de tamaño o posición del widget padre.
 *
 * @param watched Objeto que recibe el evento.
 * @param event Evento que ocurre (movimiento o redimensionamiento).
 * @return true si el evento es manejado, false en caso contrario.
 */
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

/**
 * @brief Sobrescribe el método reject() para limpiar recursos adicionales.
 *
 * Este método se encarga de realizar tareas de limpieza específicas antes de cerrar
 * el diálogo de inicio de sesión. En particular, elimina el filtro de eventos instalado
 * en el widget padre y programa la eliminación del overlay semitransparente (fondo oscuro)
 * que se aplicó al widget padre para enfocar la atención en el diálogo.
 *
 * Estas acciones aseguran que, al cerrarse el diálogo (por ejemplo, al presionar Escape),
 * no queden recursos residuales que bloqueen la interacción con la ventana principal.
 */
void RegisterWindow::reject() {
    // Verifica que exista un widget padre y remueve el filtro de eventos instalado.
    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    // Si el overlay de fondo está activo, lo elimina de forma segura y lo desasigna.
    if (backgroundOverlay) {
        backgroundOverlay->deleteLater();
        backgroundOverlay = nullptr;
    }
    // Llama al método reject() de la clase base para ejecutar el cierre del diálogo.
    QDialog::reject();
}
