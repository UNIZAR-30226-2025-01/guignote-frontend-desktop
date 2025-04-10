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
#include "friendsmessagewindow.h"
#include "userprofilewindow.h"

// Función auxiliar para crear un diálogo modal con mensaje personalizado.
// Si exitApp es verdadero, al cerrar se finaliza la aplicación.
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

// Constructor
friendswindow::friendswindow(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(900, 650);

    networkManager = new QNetworkAccessManager(this);
    setupUI();

    // Cargar información inicial
    fetchFriends();
    fetchRequests();
}

// Configuración de la UI
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

// Pestaña Amigos: muestra la lista de amigos.
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

// Pestaña Solicitudes: muestra la lista de solicitudes.
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

// Pestaña Buscar: campo de búsqueda y lista de resultados.
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



// Función para extraer el token de autenticación desde el archivo .conf
QString friendswindow::loadAuthToken() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + "/Grace Hopper/Sota, Caballo y Rey.conf";
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        createDialog(this, "No se pudo cargar el archivo de configuración.")->show();
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

// Obtiene la lista de amigos desde el servidor.
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

QWidget* friendswindow::createFriendWidget(const QJsonObject &amigo) {
    QWidget *widget = new QWidget();
    widget->setMinimumSize(300, 130);
    widget->setStyleSheet("border-radius: 10px;");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 2, 10, 2);
    layout->setSpacing(15);

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
    nameLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    infoLayout->addWidget(nameLabel);

    int wins = amigo.contains("victorias") ? amigo["victorias"].toInt() : 0;
    int losses = amigo.contains("derrotas") ? amigo["derrotas"].toInt() : 0;
    double ratio = (wins + losses > 0) ? (wins * 100.0 / (wins + losses)) : 0.0;
    QLabel *statsLabel = new QLabel(
        QString("Victorias: %1   Derrotas: %2   Ratio: %3%")
            .arg(wins).arg(losses).arg(ratio, 0, 'f', 2),
        widget
        );
    statsLabel->setStyleSheet("color: white; font-size: 18px;");
    infoLayout->addWidget(statsLabel);

    layout->addLayout(infoLayout);
    layout->addStretch();

    QString friendId;
    if (amigo.contains("id"))
        friendId = QString::number(amigo["id"].toInt());
    else if (amigo.contains("ID"))
        friendId = QString::number(amigo["ID"].toInt());

    // Botones en horizontal con más separación
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);  // Más espacio entre botones

    QPushButton *profileButton = new QPushButton("Perfil", widget);
    QPushButton *messageButton = new QPushButton("Mensajes", widget);
    QPushButton *deleteButton = new QPushButton("Eliminar", widget);

    // Estilos idénticos para todos los botones
    QString buttonStyle =
        "QPushButton {"
        "  background-color: #1D4536;"
        "  color: #F9F9F4;"
        "  font-size: 16px;"
        "  padding: 8px 15px;"
        "  border-radius: 10px;"
        "  min-width: 80px;"
        "}"
        "QPushButton:hover { background-color: #2A5C45; }";

    QString deleteButtonStyle =
        "QPushButton {"
        "  background-color: #8B3A3A;"
        "  color: #F1F1F1;"
        "  font-size: 16px;"
        "  padding: 8px 15px;"
        "  border-radius: 10px;"
        "  min-width: 80px;"
        "}"
        "QPushButton:hover { background-color: #C9B170; color: #1C1C1C; }";

    profileButton->setStyleSheet(buttonStyle);
    messageButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(deleteButtonStyle);

    buttonLayout->addWidget(profileButton);
    buttonLayout->addWidget(messageButton);
    buttonLayout->addWidget(deleteButton);

    layout->addLayout(buttonLayout);

    // Conexiones
    connect(profileButton, &QPushButton::clicked, [this]() {
        UserProfileWindow *profileWin = new UserProfileWindow(this);
        profileWin->setWindowModality(Qt::ApplicationModal);
        profileWin->move(this->geometry().center() - profileWin->rect().center());
        profileWin->show();
    });

    connect(messageButton, &QPushButton::clicked, [this]() {
        FriendsMessageWindow *messageWin = new FriendsMessageWindow(this);
        messageWin->setWindowModality(Qt::ApplicationModal);
        messageWin->move(this->geometry().center() - messageWin->rect().center());
        messageWin->show();
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


// Envía una solicitud para eliminar a un amigo.
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

// Obtiene las solicitudes de amistad desde el servidor.
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
            }
        }
        reply->deleteLater();
    });
}

void friendswindow::searchUsers() {
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


QWidget* friendswindow::createSearchResultWidget(const QJsonObject &usuario) {
    QWidget *widget = new QWidget();
    widget->setMinimumSize(300, 130);
    widget->setStyleSheet("background-color: #2a2a2a; border: 1px solid #444; border-radius: 10px;");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);

    // Avatar placeholder
    QLabel *avatarLabel = new QLabel(widget);
    avatarLabel->setFixedSize(50, 50);
    avatarLabel->setStyleSheet("background-color: #555; border-radius: 25px;");
    layout->addWidget(avatarLabel);

    // Información del usuario
    QVBoxLayout *infoLayout = new QVBoxLayout();
    // Ajustamos el margen interno del layout para dar más espacio
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
    qDebug() << "createSearchResultWidget: userId =" << userId;
    addButton->setProperty("userId", userId);
    connect(addButton, &QPushButton::clicked, this, &friendswindow::sendFriendRequest);
    layout->addWidget(addButton);

    return widget;
}



// Crea un widget para mostrar cada solicitud de amistad.
QWidget* friendswindow::createRequestWidget(const QJsonObject &solicitud) {
    QWidget *widget = new QWidget();
    widget->setMinimumSize(300, 130);
    widget->setStyleSheet("border-radius: 10px;");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10, 2, 10, 2);
    layout->setSpacing(7);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    QString nombre = solicitud["solicitante"].toString();
    QLabel *nameLabel = new QLabel(nombre, widget);
    nameLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    infoLayout->addWidget(nameLabel);

    layout->addLayout(infoLayout);
    layout->addStretch();

    QPushButton *acceptBtn = new QPushButton("Aceptar", widget);
    QString solicitudId = QString::number(solicitud["id"].toInt());
    acceptBtn->setProperty("solicitudId", solicitudId);
    acceptBtn->setStyleSheet(
        "QPushButton { background-color: #1D4536; color: #F9F9F4; font-size: 18px; padding: 10px; border-radius: 10px; }"
        "QPushButton:hover { background-color: #2A5C45; }"
        );
    connect(acceptBtn, &QPushButton::clicked, this, &friendswindow::acceptRequest);
    layout->addWidget(acceptBtn);

    QPushButton *rejectBtn = new QPushButton("Rechazar", widget);
    rejectBtn->setStyleSheet(
        "QPushButton { background-color: #8B3A3A; color: #F1F1F1; font-size: 18px; padding: 10px; border-radius: 10px; }"
        "QPushButton:hover { background-color: #C9B170; color: #1C1C1C; }"
        );
    rejectBtn->setProperty("solicitudId", solicitudId);
    connect(rejectBtn, &QPushButton::clicked, this, &friendswindow::rejectRequest);
    layout->addWidget(rejectBtn);

    widget->setLayout(layout);
    return widget;
}

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


// Acepta una solicitud de amistad.
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

// Rechaza una solicitud de amistad.
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
