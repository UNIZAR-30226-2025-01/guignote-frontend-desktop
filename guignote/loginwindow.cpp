/**
 * @file loginwindow.cpp
 * @brief Implementación de la clase LoginWindow.
 *
 * Esta clase representa la ventana de inicio de sesión en la aplicación. Permite a los usuarios
 * ingresar sus credenciales, alternar la visibilidad de la contraseña, recordar las credenciales,
 * recuperar la contraseña en caso de olvido y gestionar la comunicación con el servidor para la autenticación.
 * Además, muestra un overlay semitransparente en el widget padre para enfocar la atención en el diálogo.
 */

#include "loginwindow.h"
#include "menuwindow.h"
#include "recoverpasswordwindow.h"

// Inclusión de librerías de Qt necesarias para la interfaz y red
#include <QVBoxLayout>
#include <QMessageBox>
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
#include <QSettings>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QPropertyAnimation>
#include <QSizePolicy>
#include <QFontMetrics>
#include <qmainwindow.h>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent),
    backgroundOverlay(nullptr)
{
    // Configuración de la ventana: modal, sin bordes, con fondo personalizado y tamaño fijo.
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);

    /*
     * ------------------------------------------------------------------------------
     * AJUSTE DEL TAMAÑO DE LA VENTANA
     * ------------------------------------------------------------------------------
     */
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(400, 540);

    // Cargar la fuente personalizada para el título.
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
    mainLayout->setContentsMargins(20, 5, 20, 5);
    mainLayout->setSpacing(2);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Título de la ventana de inicio de sesión.
    QLabel *titleLabel = new QLabel("Iniciar Sesión", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Definición del estilo para los QLineEdit (campos de texto).
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Campo para ingresar el usuario o correo.
    QLineEdit *usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Usuario o Correo");
    usernameEdit->setStyleSheet(lineEditStyle);
    usernameEdit->setFixedWidth(250);
    usernameEdit->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(usernameEdit, 0, Qt::AlignCenter);

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

    /*
     * ------------------------------------------------------------------------------
     * NUEVO: Etiqueta para mostrar los errores en rojo debajo de la contraseña.
     * ------------------------------------------------------------------------------
     */
    QLabel *errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("QLabel { color: #ff4444; font-size: 14px; }");
    errorLabel->setText("");
    errorLabel->setVisible(false);

    // Mantenerlo en una sola línea con ancho y alto fijos
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setFixedWidth(250);
    //  errorLabel->setWordWrap(false); // No hace falta, pero asegura que no parta líneas
    errorLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    errorLabel->setContentsMargins(0, 5, 0, 0);

    auto setErrorTextElided = [=](const QString &text){
        QFontMetrics fm(errorLabel->font());
        QString elided = fm.elidedText(text, Qt::ElideRight, errorLabel->width());
        errorLabel->setText(elided);
        errorLabel->adjustSize();
    };
    mainLayout->addSpacing(5);
    mainLayout->addWidget(errorLabel, 0, Qt::AlignCenter);

    // Botón "¿Has olvidado tu contraseña?"
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
    mainLayout->addWidget(forgotPasswordButton);

    // CheckBox "Recordar contraseña"
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
    mainLayout->addWidget(rememberCheck);

    // Conexión para mostrar la ventana de recuperación de contraseña.
    connect(forgotPasswordButton, &QPushButton::clicked, this, [=]() {
        RecoverPasswordWindow *recoverWin = new RecoverPasswordWindow(this);
        // Centrar la ventana de recuperación respecto al diálogo actual.
        recoverWin->move(this->geometry().center() - recoverWin->rect().center());
        recoverWin->show();
    });

    // Botón "Iniciar Sesión" con su estilo personalizado.
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

    /*
     * ------------------------------------------------------------------------------
     * NUEVO: Función para sacudir el BOTÓN
     * ------------------------------------------------------------------------------
     */
    auto shakeWidget = [=](QWidget *widget) {
        QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
        animation->setDuration(200);
        animation->setKeyValueAt(0,       widget->pos());
        animation->setKeyValueAt(0.2,     widget->pos() + QPoint(10, 0));
        animation->setKeyValueAt(0.4,     widget->pos() + QPoint(-10, 0));
        animation->setKeyValueAt(0.6,     widget->pos() + QPoint(10, 0));
        animation->setKeyValueAt(0.8,     widget->pos() + QPoint(-10, 0));
        animation->setKeyValueAt(1,       widget->pos());
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    };

    // Conexión para realizar la autenticación mediante una petición HTTP POST.
    connect(loginButton, &QPushButton::clicked, [=]() {
        QString userOrEmail = usernameEdit->text().trimmed();
        QString contrasegna = passwordEdit->text();

        // Validar que ambos campos contengan datos.
        if (userOrEmail.isEmpty() || contrasegna.isEmpty()) {
            // Se muestra el error en el label rojo y se sacude el BOTÓN de iniciar sesión.
            setErrorTextElided("Faltan campos.");
            errorLabel->setVisible(true);
            shakeWidget(loginButton);
            return;
        }

        // Crear el objeto JSON con las credenciales.
        QJsonObject json;
        if (userOrEmail.contains('@')) {
            json["correo"] = userOrEmail;
        } else {
            json["nombre"] = userOrEmail;
        }
        json["contrasegna"] = contrasegna;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        // Configuración de la petición POST a la URL de autenticación.
        QUrl url("http://188.165.76.134:8000/usuarios/iniciar_sesion/");
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
                    // Si se recibe un token, se procede a abrir el menú en el mismo espacio que MainWindow.
                    if (respObj.contains("token")) {
                        QString token = respObj["token"].toString();
                        qDebug() << "Token recibido:" << token;
                        // Almacenamos el token de forma persistente
                        QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
                        settings.setValue("auth/token", token);

                        MenuWindow *menuWin = new MenuWindow();
                        // Si existe una ventana padre (MainWindow), se reemplaza el widget central por el menú.
                        if (this->parentWidget()) {
                            QMainWindow *mainWin = qobject_cast<QMainWindow*>(this->parentWidget());
                            if (mainWin) {
                                mainWin->setCentralWidget(menuWin);
                            }
                            this->close();
                        } else {
                            // Si no hay ventana padre, se muestra el menú en una nueva ventana.
                            menuWin->move(this->geometry().center() - menuWin->rect().center());
                            menuWin->show();
                            this->close();
                        }
                    } else if (respObj.contains("error")) {
                        qWarning() << "Error:" << respObj["error"].toString();
                        setErrorTextElided(respObj["error"].toString());
                        errorLabel->setVisible(true);
                        shakeWidget(loginButton);
                    } else {
                        qWarning() << "Respuesta inesperada:" << responseData;
                        setErrorTextElided("Error desconocido en la respuesta.");
                        errorLabel->setVisible(true);
                        shakeWidget(loginButton);
                    }
                }
            } else if (statusCode == 404) {
                // Mostrar mensaje de error estético: usuario no encontrado.
                qDebug() << "Qué error tengo: " << reply->errorString();
                setErrorTextElided("Usuario no encontrado.");
                errorLabel->setVisible(true);
                shakeWidget(loginButton);
            }
            else {
                qWarning() << "Error en la petición:" << reply->errorString();
                setErrorTextElided("Error en la petición.");
                errorLabel->setVisible(true);
                shakeWidget(loginButton);
            }
            reply->deleteLater();
            manager->deleteLater();
        });

        // Enviar la petición POST.
        manager->post(request, data);
    });


    // Botón "Volver" para cerrar la ventana de inicio de sesión.
    QPushButton *backButton = new QPushButton("Volver", this);
    backButton->setStyleSheet(buttonStyle);
    backButton->setFixedSize(200, 50);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    connect(backButton, &QPushButton::clicked, this, &QDialog::close);

    // Layout para el enlace que dirige a la ventana de registro.
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

    // Conexión para cerrar la ventana actual y emitir la señal de solicitud de registro.
    connect(regButtonLink, &QPushButton::clicked, [=]() {
        this->close();
        emit openRegisterRequested();
    });
}

/**
 * @brief Destructor de la ventana de inicio de sesión.
 */
LoginWindow::~LoginWindow()
{
}

/**
 * @brief Evento que se ejecuta al mostrar la ventana.
 *
 * Crea un overlay semitransparente en el widget padre para resaltar el diálogo,
 * instala un filtro de eventos para actualizar la posición del diálogo y centra el mismo en el padre.
 *
 * @param event Evento de tipo QShowEvent.
 */
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

/**
 * @brief Evento que se ejecuta al cerrar la ventana.
 *
 * Elimina el overlay de fondo y remueve el filtro de eventos del widget padre.
 *
 * @param event Evento de tipo QCloseEvent.
 */
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

/**
 * @brief Filtra eventos del widget padre.
 *
 * Se encarga de reajustar la posición del diálogo y del overlay cuando el widget padre
 * se mueve o redimensiona.
 *
 * @param watched Objeto que recibe el evento.
 * @param event Evento que ocurre (movimiento o redimensionamiento).
 * @return true si el evento se procesa, false en caso contrario.
 */
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
void LoginWindow::reject() {
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

