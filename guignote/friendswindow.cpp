/**
 * @file friendswindow.cpp
 * @brief Implementación de la clase friendswindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de la clase friendswindow, encargada de
 * gestionar la interfaz de usuario para el menú de amigos, incluyendo
 * visualización de amigos, solicitudes y búsqueda de usuarios,
 * así como gestión de peticiones REST y WebSocket.
 */


#include "friendswindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QDebug>
#include <QPointer>
#include <QPainter>
#include <QPainterPath>
#include "friendsmessagewindow.h"
#include "userprofilewindow.h"

/**
 * @brief Crea un diálogo modal personalizado con mensaje.
 * @param parent Widget padre.
 * @param message Texto a mostrar en el diálogo.
 * @param exitApp True para cerrar la aplicación al cerrar el diálogo.
 * @return Puntero al QDialog creado.
 */

static QDialog* createDialog(QWidget *parent, const QString &message, bool exitApp = false) {
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog->setStyleSheet("QDialog { background-color: #171718; border-radius: 5px; padding: 20px; }");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialog);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(4, 4);
    dialog->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QLabel *label = new QLabel(message, dialog);
    label->setWordWrap(true);
    label->setStyleSheet("color: white; font-size: 16px;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QPushButton *okButton = new QPushButton("OK", dialog);
    okButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; color: #171718; border-radius: 15px;"
        "font-size: 20px; font-weight: bold; padding: 12px 25px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    okButton->setFixedSize(100, 40);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    QObject::connect(okButton, &QPushButton::clicked, [dialog, exitApp]() {
        dialog->close();
        if (exitApp)
            qApp->quit();
    });

    dialog->adjustSize();
    dialog->move(parent->geometry().center() - dialog->rect().center());
    return dialog;
}

/**
 * @brief Constructor de la clase friendswindow.
 * @param userKey Clave identificadora del usuario.
 * @param parent Widget padre.
 */

friendswindow::friendswindow(const QString &userKey, QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(900, 650);

    this->userKey = userKey;

    networkManager = new QNetworkAccessManager(this);
    setupUI();

    // Cargar información inicial
    fetchFriends();
    fetchRequests();
}

/**
 * @brief Configura la interfaz de usuario principal.
 */

void friendswindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // Encabezado: Título y botón de cierre.
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Menú de Amigos", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");

    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(22, 22));
    closeButton->setFixedSize(35, 35);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 17px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // QTabWidget con las pestañas: Amigos, Solicitudes y Buscar.
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #555; border-radius: 10px; }"
        "QTabBar { margin-left: 15px; }"
        "QTabBar::tab {"
        "  background-color: #C2C7B0;"
        "  color: #203F31;"
        "  padding: 12px;"
        "  font-size: 16px;"
        "  border-top-left-radius: 10px;"
        "  border-top-right-radius: 10px;"
        "  margin-right: 10px;"
        "}"
        "QTabBar::tab:selected { background-color: #1D4536; color: #F9F9F4; }"
        );
    tabWidget->addTab(createFriendsTab(), "Amigos");
    tabWidget->addTab(createRequestsTab(), "Solicitudes");
    tabWidget->addTab(createSearchTab(), "Buscar");



    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}

/**
 * @brief Crea la pestaña "Amigos".
 * @return Puntero al QWidget de la pestaña.
 */

QWidget* friendswindow::createFriendsTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    friendsListWidget = new QListWidget(page);
    friendsListWidget->setStyleSheet(
        "QListWidget { background-color: #222; color: white; border-radius: 10px; padding: 8px; font-size: 18px; }"
        );
    friendsListWidget->setSpacing(50);
    layout->addWidget(friendsListWidget);
    page->setLayout(layout);
    return page;
}

/**
 * @brief Crea la pestaña "Solicitudes".
 * @return Puntero al QWidget de la pestaña.
 */

QWidget* friendswindow::createRequestsTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    requestsListWidget = new QListWidget(page);
    requestsListWidget->setStyleSheet(
        "QListWidget { background-color: #222; border-radius: 10px; padding: 8px; }"
        );
    requestsListWidget->setSpacing(50);
    layout->addWidget(requestsListWidget);
    page->setLayout(layout);
    return page;
}

/**
 * @brief Crea la pestaña "Buscar" con campo de búsqueda.
 * @return Puntero al QWidget de la pestaña.
 */

QWidget* friendswindow::createSearchTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLineEdit = new QLineEdit(page);
    searchLineEdit->setPlaceholderText("Buscar usuarios...");
    searchLineEdit->setStyleSheet(
        "QLineEdit { background-color: #222; color: white; border: 1px solid #555; border-radius: 15px; padding: 10px; font-size: 18px; }"
        "QLineEdit:focus { border: 1px solid #888; }"
        );
    searchLineEdit->setMinimumHeight(45);
    connect(searchLineEdit, &QLineEdit::textChanged, this, &friendswindow::searchUsers);
    searchLayout->addWidget(searchLineEdit);
    layout->addLayout(searchLayout);

    searchResultsListWidget = new QListWidget(page);
    searchResultsListWidget->setStyleSheet(
        "QListWidget { background-color: #222; border-radius: 10px; padding: 8px; }"
        );
    searchResultsListWidget->setSpacing(50);
    layout->addWidget(searchResultsListWidget);
    layout->setContentsMargins(20, 0, 20, 12);
    page->setLayout(layout);

    // Llamamos a searchUsers para cargar todos los usuarios al abrir la pestaña.
    searchUsers();

    return page;
}


/**
 * @brief Obtiene el token de autenticación desde el archivo de configuración.
 * @return Token como QString, o cadena vacía si falla.
 */

QString friendswindow::loadAuthToken() {

    qDebug() << "VALOR DE userKey EN loadAuthToken: " << userKey;

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);

    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        createDialog(this, "No se pudo cargar el archivo de configuración. usrKey = " + userKey)->show();
        return "";
    }
    QString token;
    while (!configFile.atEnd()) {
        QString line = configFile.readLine().trimmed();
        if (line.startsWith("token=")) {
            token = line.mid(QString("token=").length()).trimmed();
            break;
        }
    }
    configFile.close();
    if (token.isEmpty()) {
        createDialog(this, "No se encontró el token en el archivo de configuración.")->show();
    }
    return token;
}

/**
 * @brief Descarga la lista de amigos desde el servidor y la muestra.
 */

void friendswindow::fetchFriends() {
    QString token = loadAuthToken();
    if (token.isEmpty()) return;

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/usuarios/obtener_amigos/"));
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            reply->deleteLater();
            return;
        }
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("amigos")) {
                friendsListWidget->clear();
                QJsonArray amigos = obj["amigos"].toArray();
                for (const QJsonValue &value : amigos) {
                    QJsonObject amigo = value.toObject();
                    QWidget *friendCard = createFriendWidget(amigo);
                    QListWidgetItem *item = new QListWidgetItem(friendsListWidget);
                    item->setSizeHint(friendCard->sizeHint());
                    friendsListWidget->addItem(item);
                    friendsListWidget->setItemWidget(item, friendCard);
                }
            }
        }
        reply->deleteLater();
    });
}

/**
 * @brief Crea el widget de tarjeta de un amigo.
 * @param amigo Objeto JSON con datos del amigo.
 * @return Puntero al QWidget que representa la tarjeta.
 */

QWidget* friendswindow::createFriendWidget(const QJsonObject &amigo) {
    QWidget *widget = new QWidget();
    widget->setMinimumSize(300, 130);  // Tamaño mínimo fijo
    widget->setStyleSheet("border-radius: 10px;");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 2, 10, 2);
    layout->setSpacing(15);

    // Layout para la foto de perfil y el nombre
    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(5);  // Reducir el espacio entre la foto y el nombre

    // Icon placeholder
    int pfpSize = 100;
    QString avatarUrl = amigo["imagen"].toString();  // Obtener la URL de la imagen
    Icon *avatarIcon = new Icon(this);
    avatarIcon->setHoverEnabled(false);

    // Descargar la imagen de perfil y asignarla al Icon
    if (!avatarUrl.isEmpty()) {
        downloadAndSetAvatar(avatarUrl, avatarIcon);
    } else {
        // Si no se proporciona imagen, usar una imagen predeterminada
        QString imagePath = ":/icons/profile.png";
        QPixmap circularImage = createCircularImage(imagePath, pfpSize);
        avatarIcon->setPixmapImg(circularImage, pfpSize, pfpSize);
    }

    leftLayout->addWidget(avatarIcon);
    leftLayout->addSpacing(5);  // Reducir la distancia entre la imagen y el nombre

    // Información del amigo
    QVBoxLayout *infoLayout = new QVBoxLayout();
    QString nombre = "Nombre no disponible";
    if (amigo.contains("nombre"))
        nombre = amigo["nombre"].toString();
    else if (amigo.contains("Nombre"))
        nombre = amigo["Nombre"].toString();
    else if (amigo.contains("username"))
        nombre = amigo["username"].toString();
    qDebug() << "El nombre del usuario es" << nombre;
    QLabel *nameLabel = new QLabel(nombre, widget);
    nameLabel->setWordWrap(true);  // Permite el ajuste de texto en varias líneas
    nameLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    infoLayout->addWidget(nameLabel);

    int wins = amigo.contains("victorias") ? amigo["victorias"].toInt() : 0;
    int losses = amigo.contains("derrotas") ? amigo["derrotas"].toInt() : 0;
    double ratio = (wins + losses > 0) ? (wins * 100.0 / (wins + losses)) : 0.0;
    QLabel *statsLabel = new QLabel(
        QString("Victorias: %1   Derrotas: %2   Ratio: %3%")
            .arg(wins).arg(losses).arg(ratio, 0, 'f', 2),
        widget
        );
    statsLabel->setStyleSheet("color: white; font-size: 16px;");
    infoLayout->addWidget(statsLabel);

    leftLayout->addLayout(infoLayout);
    layout->addLayout(leftLayout);
    layout->addStretch();

    QString friendId;
    if (amigo.contains("id"))
        friendId = QString::number(amigo["id"].toInt());
    else if (amigo.contains("ID"))
        friendId = QString::number(amigo["ID"].toInt());

    // Botones en horizontal con más separación
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);  // Más espacio entre botones
    buttonLayout->setContentsMargins(10, 5, 10, 5);  // Ajustar márgenes de los botones

    // Botones más pequeños
    QPushButton *profileButton = new QPushButton("Perfil", widget);
    QPushButton *messageButton = new QPushButton("Mensajes", widget);
    QPushButton *deleteButton = new QPushButton("Eliminar", widget);

    // Establecer tamaño más pequeño de los botones
    profileButton->setFixedSize(70, 30);
    messageButton->setFixedSize(70, 30);
    deleteButton->setFixedSize(70, 30);

    // Estilos para los botones pequeños
    QString buttonStyle =
        "QPushButton {"
        "  background-color: #1D4536;"
        "  color: #F9F9F4;"
        "  font-size: 14px;"
        "  padding: 5px 10px;"
        "  border-radius: 10px;"
        "  min-width: 70px;"
        "}"
        "QPushButton:hover { background-color: #2A5C45; }";

    QString deleteButtonStyle =
        "QPushButton {"
        "  background-color: #8B3A3A;"
        "  color: #F1F1F1;"
        "  font-size: 14px;"
        "  padding: 5px 10px;"
        "  border-radius: 10px;"
        "  min-width: 70px;"
        "}"
        "QPushButton:hover { background-color: #C9B170; color: #1C1C1C; }";

    profileButton->setStyleSheet(buttonStyle);
    messageButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(deleteButtonStyle);

    buttonLayout->addWidget(profileButton);
    buttonLayout->addWidget(messageButton);
    buttonLayout->addWidget(deleteButton);

    layout->addLayout(buttonLayout);

    // Conexiones para los botones
    connect(profileButton, &QPushButton::clicked, [this, friendId]() {
        UserProfileWindow *profileWin = new UserProfileWindow(this, userKey, friendId);
        profileWin->setWindowModality(Qt::ApplicationModal);
        profileWin->move(this->geometry().center() - profileWin->rect().center());
        profileWin->show();
    });

    static QMap<QString, FriendsMessageWindow*> openChats;

    connect(messageButton, &QPushButton::clicked, [this, friendId, nombre]() {
        FriendsMessageWindow *w;
        if (!openChats.contains(friendId)) {
            w = new FriendsMessageWindow(userKey, friendId, nombre, this);
            openChats.insert(friendId, w);
        } else {
            w = openChats.value(friendId);
        }
        w->show();    // si estaba oculta, la muestra
        w->raise();   // y la pone encima
    });

    connect(deleteButton, &QPushButton::clicked, [this, friendId]() {
        QDialog *confirmDialog = new QDialog(this);
        confirmDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        confirmDialog->setModal(true);
        confirmDialog->setStyleSheet(
            "QDialog {"
            "  background-color: #171718;"
            "  border-radius: 5px;"
            "  padding: 20px;"
            "}"
            );

        QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(confirmDialog);
        dialogShadow->setBlurRadius(10);
        dialogShadow->setColor(QColor(0, 0, 0, 80));
        dialogShadow->setOffset(4, 4);
        confirmDialog->setGraphicsEffect(dialogShadow);

        QVBoxLayout *dialogLayout = new QVBoxLayout(confirmDialog);
        QLabel *confirmLabel = new QLabel("¿Está seguro que desea eliminar a este amigo?", confirmDialog);
        confirmLabel->setWordWrap(true);
        confirmLabel->setStyleSheet("color: white; font-size: 16px;");
        confirmLabel->setAlignment(Qt::AlignCenter);
        dialogLayout->addWidget(confirmLabel);

        QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
        QPushButton *yesButton = new QPushButton("Sí", confirmDialog);
        QPushButton *noButton = new QPushButton("No", confirmDialog);
        yesButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #1D4536;"
            "  color: #F9F9F4;"
            "  padding: 10px 25px;"
            "  border-radius: 10px;"
            "}"
            );
        noButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #8B3A3A;"
            "  color: #F1F1F1;"
            "  padding: 10px 25px;"
            "  border-radius: 10px;"
            "}"
            );
        dialogButtonLayout->addWidget(yesButton);
        dialogButtonLayout->addWidget(noButton);
        dialogLayout->addLayout(dialogButtonLayout);

        connect(yesButton, &QPushButton::clicked, [this, confirmDialog, friendId]() {
            removeFriend(friendId);
            confirmDialog->close();
        });
        connect(noButton, &QPushButton::clicked, confirmDialog, &QDialog::close);

        confirmDialog->adjustSize();
        confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        confirmDialog->show();
    });

    widget->setLayout(layout);
    return widget;
}

/**
 * @brief Envía petición para eliminar a un amigo.
 * @param friendId ID del amigo a eliminar.
 */

void friendswindow::removeFriend(const QString &friendId) {
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "removeFriend: token vacío.";
        return;
    }

    QUrl url("http://188.165.76.134:8000/usuarios/eliminar_amigo/");
    QUrlQuery query;
    query.addQueryItem("amigo_id", friendId);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth", token.toUtf8());

    QNetworkReply *reply = networkManager->sendCustomRequest(request, "DELETE");
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            reply->deleteLater();
            return;
        } else if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Amigo eliminado correctamente.";
            fetchFriends();
        } else {
            qDebug() << "Error en removeFriend, código:" << statusCode;
        }
        reply->deleteLater();
    });
}

/**
 * @brief Obtiene la lista de solicitudes de amistad del servidor.
 */

void friendswindow::fetchRequests() {
    QString token = loadAuthToken();
    if (token.isEmpty()) return;

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/usuarios/listar_solicitudes_amistad/"));
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            reply->deleteLater();
            return;
        }
        QByteArray response = reply->readAll();
        qDebug() << "fetchRequests response:" << response;
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("solicitudes")) {
                requestsListWidget->clear();
                QJsonArray solicitudes = obj["solicitudes"].toArray();
                for (const QJsonValue &value : solicitudes) {
                    QJsonObject solicitud = value.toObject();
                    QListWidgetItem *item = new QListWidgetItem(requestsListWidget);
                    QWidget *widget = createRequestWidget(solicitud);
                    item->setSizeHint(widget->sizeHint());
                    requestsListWidget->addItem(item);
                    requestsListWidget->setItemWidget(item, widget);
                }
                int solicitudesCount = solicitudes.size();
                emit friendRequestsCountChanged(solicitudesCount);
            }
        }
        reply->deleteLater();
    });
}

/**
 * @brief Realiza búsqueda de usuarios según el texto ingresado.
 * Dispara REST y actualiza la lista de resultados.
 */

void friendswindow::searchUsers() {
    qDebug() << "VALOR DE USERKEY EN searchUsers:" << userKey;
    QString searchText = searchLineEdit->text().trimmed();
    qDebug() << "searchUsers triggered with text:" << searchText;
    currentSearchQuery = searchText;

    // Se elimina el siguiente bloque para que, aun cuando searchText esté vacío, se realice la búsqueda:
    // if (searchText.isEmpty()) {
    //     searchResultsListWidget->clear();
    //     return;
    // }

    searchResultsListWidget->clear();

    QString token = loadAuthToken();
    if (token.isEmpty()) return;

    QUrl url("http://188.165.76.134:8000/usuarios/buscar_usuarios/");
    QUrlQuery query;
    query.addQueryItem("nombre", searchText);
    query.addQueryItem("incluir_amigos", "false");
    query.addQueryItem("incluir_me", "false");
    query.addQueryItem("incluir_pendientes", "true");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, queryText = searchText]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            reply->deleteLater();
            return;
        }
        if (currentSearchQuery != queryText) {
            reply->deleteLater();
            return;
        }
        searchResultsListWidget->clear();
        QByteArray response = reply->readAll();
        qDebug() << "searchUsers response:" << response;
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("usuarios")) {
                QJsonArray usuarios = obj["usuarios"].toArray();
                for (const QJsonValue &value : usuarios) {
                    QJsonObject usuario = value.toObject();
                    qDebug() << "Usuario recibido:" << usuario;
                    QWidget *widget = createSearchResultWidget(usuario);
                    QListWidgetItem *item = new QListWidgetItem(searchResultsListWidget);
                    item->setSizeHint(widget->sizeHint());
                    searchResultsListWidget->addItem(item);
                    searchResultsListWidget->setItemWidget(item, widget);
                }
            }
        }
        reply->deleteLater();
    });
}

/**
 * @brief Crea el widget de resultado de búsqueda de usuario.
 * @param usuario Objeto JSON con datos del usuario.
 * @return Puntero al QWidget de resultado.
 */

QWidget* friendswindow::createSearchResultWidget(const QJsonObject &usuario) {
    QWidget *widget = new QWidget();
    widget->setMinimumSize(300, 130);
    widget->setStyleSheet("background-color: #2a2a2a; border: 1px solid #444; border-radius: 10px;");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);

    // Icon placeholder
    int pfpSize = 100;
    QString imagePath = ":/icons/profile.png";
    QPixmap circularImage = createCircularImage(imagePath, pfpSize);

    Icon *avatarIcon = new Icon(this);
    avatarIcon->setHoverEnabled(false);
    avatarIcon->setPixmapImg(circularImage, pfpSize, pfpSize);
    layout->addWidget(avatarIcon);

    // Información del usuario
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setContentsMargins(5, 5, 5, 5);

    QString nombre;
    if (usuario.contains("nombre"))
        nombre = usuario["nombre"].toString();
    else if (usuario.contains("Nombre"))
        nombre = usuario["Nombre"].toString();
    else if (usuario.contains("username"))
        nombre = usuario["username"].toString();
    else
        nombre = "Nombre no disponible";

    QLabel *nameLabel = new QLabel(nombre, widget);
    nameLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold; padding-top: 5px; padding-bottom: 5px;");
    infoLayout->addWidget(nameLabel);

    int wins = usuario.contains("victorias") ? usuario["victorias"].toInt() : 0;
    int losses = usuario.contains("derrotas") ? usuario["derrotas"].toInt() : 0;
    double ratio = (wins + losses > 0) ? (wins * 100.0 / (wins + losses)) : 0.0;
    QLabel *statsLabel = new QLabel(
        QString("Victorias: %1   Derrotas: %2   Ratio: %3%")
            .arg(wins).arg(losses).arg(ratio, 0, 'f', 2),
        widget
        );
    statsLabel->setStyleSheet("color: white; font-size: 18px; padding-top: 5px; padding-bottom: 5px;");
    infoLayout->addWidget(statsLabel);

    layout->addLayout(infoLayout);
    layout->addStretch();

    // Botón para agregar amigo
    QPushButton *addButton = new QPushButton("Agregar amigo", widget);
    addButton->setStyleSheet("background-color: #1D4536; color: #F9F9F4; font-size: 18px; padding: 10px; border-radius: 10px;");
    QString userId;
    if (usuario.contains("id"))
        userId = QString::number(usuario["id"].toInt());
    else if (usuario.contains("ID"))
        userId = QString::number(usuario["ID"].toInt());
    else
        userId = "";
    addButton->setProperty("userId", userId);
    connect(addButton, &QPushButton::clicked, this, [=]() {
        this->sendFriendRequest();
    });
    layout->addWidget(addButton);

    // Descargar la imagen de avatar y asignarla al Icon
    QString avatarUrl = usuario.contains("imagen") ? usuario["imagen"].toString() : "";
    if (!avatarUrl.isEmpty()) {
        downloadAndSetAvatar(avatarUrl, avatarIcon);
    }

    return widget;
}

/**
 * @brief Descarga una imagen y la asigna recortada como avatar circular.
 * @param imageUrl URL de la imagen.
 * @param avatarIcon Puntero al Icon donde mostrarla.
 */
void friendswindow::downloadAndSetAvatar(const QString &imageUrl, Icon *avatarIcon) {
    if (!avatarIcon) {
        qDebug() << "imagen perfil: avatarIcon es nullptr.";
        return;
    }
    if (imageUrl.isEmpty() || !QUrl(imageUrl).isValid()) {
        qDebug() << "imagen perfil: la URL de la imagen es inválida o está vacía:" << imageUrl;
        return;
    }
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request{QUrl(imageUrl)};
    QNetworkReply *reply = manager->get(request);
    QPointer<friendswindow> weakSelf = this;
    QPointer<Icon> weakAvatarIcon = avatarIcon;
    connect(reply, &QNetworkReply::finished, this, [weakSelf, weakAvatarIcon, reply, imageUrl]() {
        if (!weakSelf || !weakAvatarIcon) {
            qDebug() << "imagen perfil: error destrucción anticipada.";
            if (reply) reply->deleteLater();
            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imgData = reply->readAll();
            QPixmap pixmap;
            if (pixmap.loadFromData(imgData)) {
                int size = 100;
                QPixmap circular = weakSelf->createCircularImage(pixmap, size);
                if (weakAvatarIcon) weakAvatarIcon->setPixmapImg(circular, size, size);
            } else {
                qDebug() << "Error al cargar la imagen desde QByteArray para" << imageUrl;
            }
        } else {
            qDebug() << "Error al descargar la imagen" << imageUrl << ":" << reply->errorString();
        }
        reply->deleteLater();
    });
}

/**
 * @brief Convierte un QPixmap en una versión circular de tamaño dado.
 * @param src QPixmap de origen.
 * @param size Dimensión del lado del círculo.
 * @return QPixmap circular resultante.
 */

QPixmap friendswindow::createCircularImage(const QPixmap &src, int size) {
    // Escalamos a un cuadrado de “size × size” y recortamos el exceso
    QPixmap scaled = src.scaled(size, size,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);
    QPixmap circular(size, size);
    circular.fill(Qt::transparent);

    QPainter painter(&circular);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, size, size, scaled);

    return circular;
}

/**
 * @brief Crea el widget de una solicitud de amistad.
 * @param solicitud Objeto JSON con datos de la solicitud.
 * @return Puntero al QWidget de la solicitud.
 */

QWidget* friendswindow::createRequestWidget(const QJsonObject &solicitud) {
    QWidget *widget = new QWidget();
    widget->setMinimumSize(300, 130);
    widget->setStyleSheet("border-radius: 10px;");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 2, 10, 2);
    layout->setSpacing(7);

    // Layout para la foto de perfil y el nombre
    QHBoxLayout *leftLayout = new QHBoxLayout();

    // Icon placeholder
    int pfpSize = 100;
    QString avatarUrl = solicitud["imagen"].toString();  // Obtener la URL de la imagen
    Icon *avatarIcon = new Icon(this);
    avatarIcon->setHoverEnabled(false);

    // Descargar la imagen de perfil y asignarla al Icon
    if (!avatarUrl.isEmpty()) {
        downloadAndSetAvatar(avatarUrl, avatarIcon);
    } else {
        // Si no se proporciona imagen, usar una imagen predeterminada
        QString imagePath = ":/icons/profile.png";
        QPixmap circularImage = createCircularImage(imagePath, pfpSize);
        avatarIcon->setPixmapImg(circularImage, pfpSize, pfpSize);
    }

    leftLayout->addWidget(avatarIcon);
    leftLayout->addSpacing(10);  // Espacio entre la foto de perfil y el nombre

    // Información del solicitante
    QVBoxLayout *infoLayout = new QVBoxLayout();
    QString nombre = solicitud["solicitante"].toString();
    QLabel *nameLabel = new QLabel(nombre, widget);
    nameLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    infoLayout->addWidget(nameLabel);

    leftLayout->addLayout(infoLayout);
    layout->addLayout(leftLayout);
    layout->addStretch();  // Espacio para el resto de los widgets

    // Botón Aceptar
    QPushButton *acceptBtn = new QPushButton("Aceptar", widget);
    QString solicitudId = QString::number(solicitud["id"].toInt());
    acceptBtn->setProperty("solicitudId", solicitudId);
    acceptBtn->setStyleSheet(
        "QPushButton { background-color: #1D4536; color: #F9F9F4; font-size: 18px; padding: 10px; border-radius: 10px; }"
        "QPushButton:hover { background-color: #2A5C45; }"
        );
    connect(acceptBtn, &QPushButton::clicked, this, [=]() {
        this->acceptRequest();
    });
    layout->addWidget(acceptBtn);

    // Botón Rechazar
    QPushButton *rejectBtn = new QPushButton("Rechazar", widget);
    rejectBtn->setStyleSheet(
        "QPushButton { background-color: #8B3A3A; color: #F1F1F1; font-size: 18px; padding: 10px; border-radius: 10px; }"
        "QPushButton:hover { background-color: #C9B170; color: #1C1C1C; }"
        );
    rejectBtn->setProperty("solicitudId", solicitudId);
    connect(rejectBtn, &QPushButton::clicked, this, [=]() {
        this->rejectRequest();
    });
    layout->addWidget(rejectBtn);

    widget->setLayout(layout);
    return widget;
}

/**
 * @brief Envía una solicitud de amistad al usuario indicado.
 */

void friendswindow::sendFriendRequest() {
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        qDebug() << "sendFriendRequest: El sender no es un QPushButton.";
        return;
    }
    QString userId = button->property("userId").toString();
    if (userId.isEmpty()) {
        qDebug() << "sendFriendRequest: userId está vacío.";
        return;
    }
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "sendFriendRequest: token vacío.";
        return;
    }
    qDebug() << "Enviando solicitud de amistad para userId:" << userId;

    QUrl url("http://188.165.76.134:8000/usuarios/enviar_solicitud_amistad/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth", token.toUtf8());

    QJsonObject json;
    json["destinatario_id"] = userId;
    QJsonDocument doc(json);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, [this, reply, button, userId]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Solicitud enviada correctamente para userId:" << userId;
            // Actualizamos el botón para indicar que la solicitud fue enviada
            button->setText("Solicitud Enviada");
            button->setStyleSheet("background-color: #C2C7B0; color: #203F31; font-size: 18px; padding: 10px; border-radius: 10px;");
            button->setEnabled(false);
            createDialog(this, "Se ha enviado la solicitud de amistad.")->show();
        } else {
            qDebug() << "Error al enviar la solicitud para userId:" << userId << " Error:" << reply->errorString();
            createDialog(this, "No se pudo enviar la solicitud de amistad.")->show();
        }
        reply->deleteLater();
    });
}


/**
 * @brief Acepta la solicitud de amistad seleccionada.
 */

void friendswindow::acceptRequest() {
    QObject *senderObj = sender();
    if (!senderObj) return;
    QString solicitudId = senderObj->property("solicitudId").toString();
    if (solicitudId.isEmpty()) return;
    QString token = loadAuthToken();
    if (token.isEmpty()) return;

    QUrl url("http://188.165.76.134:8000/usuarios/aceptar_solicitud_amistad/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth", token.toUtf8());

    QJsonObject json;
    json["solicitud_id"] = solicitudId;
    QJsonDocument doc(json);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            reply->deleteLater();
            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            createDialog(this, "Has aceptado la solicitud de amistad.")->show();
            fetchRequests();
            fetchFriends();
        } else {
            QMessageBox::warning(this, "Error", "No se pudo aceptar la solicitud.");
        }
        reply->deleteLater();
    });
}

/**
 * @brief Rechaza la solicitud de amistad seleccionada.
 */

void friendswindow::rejectRequest() {
    QObject *senderObj = sender();
    if (!senderObj) return;
    QString solicitudId = senderObj->property("solicitudId").toString();
    if (solicitudId.isEmpty()) return;
    QString token = loadAuthToken();
    if(token.isEmpty()) return;

    QUrl url("http://188.165.76.134:8000/usuarios/denegar_solicitud_amistad/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth", token.toUtf8());

    QJsonObject json;
    json["solicitud_id"] = solicitudId;
    QJsonDocument doc(json);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if(reply->error() == QNetworkReply::NoError) {
            createDialog(this, "Solicitud de amistad rechazada.")->show();
            fetchRequests();
        } else {
            QMessageBox::warning(this, "Error", "No se pudo rechazar la solicitud.");
        }
        reply->deleteLater();
    });
}
