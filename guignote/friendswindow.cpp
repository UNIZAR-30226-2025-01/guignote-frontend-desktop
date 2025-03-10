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

    // Encabezado: título y botón de cierre
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
        "QTabBar::tab { background-color: #222; color: white; padding: 12px; font-size: 16px; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
        "QTabBar::tab:selected { background-color: #4CAF50; }"
        "QTabWidget::pane { border: 1px solid #555; border-radius: 10px; }"
        );

    tabWidget->addTab(createFriendsTab(), "Amigos");
    tabWidget->addTab(createRequestsTab(), "Solicitudes");
    tabWidget->addTab(createSearchTab(), "Buscar");

    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}

// Pestaña Amigos: lista simple de amigos.
QWidget* friendswindow::createFriendsTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    friendsListWidget = new QListWidget(page);
    friendsListWidget->setStyleSheet(
        "QListWidget { background-color: #222; color: white; border-radius: 10px; padding: 8px; font-size: 18px; }"
        );
    friendsListWidget->setSpacing(10);
    layout->addWidget(friendsListWidget);
    page->setLayout(layout);
    return page;
}

// Pestaña Solicitudes: lista de solicitudes con widgets personalizados.
QWidget* friendswindow::createRequestsTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    requestsListWidget = new QListWidget(page);
    requestsListWidget->setStyleSheet(
        "QListWidget { background-color: #222; border-radius: 10px; padding: 8px; }"
        );
    requestsListWidget->setSpacing(10);
    layout->addWidget(requestsListWidget);
    page->setLayout(layout);
    return page;
}

// Pestaña Buscar: campo de búsqueda y listado de resultados con estadísticas y botón de agregar.
QWidget* friendswindow::createSearchTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    // Ajustamos los márgenes del layout a valores iguales arriba y abajo

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLineEdit = new QLineEdit(page);
    searchLineEdit->setPlaceholderText("Buscar usuarios...");
    searchLineEdit->setStyleSheet(
        "QLineEdit { background-color: #222; color: white; border: 1px solid #555; border-radius: 15px; padding: 10px; font-size: 18px; }"
        "QLineEdit:focus { border: 1px solid #888; }"
        );
    searchLineEdit->setMinimumHeight(45);

    // Actualizar búsqueda automáticamente al cambiar el texto.
    connect(searchLineEdit, &QLineEdit::textChanged, this, &friendswindow::searchUsers);
    searchLayout->addWidget(searchLineEdit);
    layout->addLayout(searchLayout);

    searchResultsListWidget = new QListWidget(page);
    searchResultsListWidget->setStyleSheet(
        "QListWidget { background-color: #222; border-radius: 10px; padding: 8px; }"
        );
    searchResultsListWidget->setSpacing(50);
    layout->addWidget(searchResultsListWidget);
    layout->setContentsMargins(20,0,20,12);
    page->setLayout(layout);
    return page;
}

// Función para extraer el token de autenticación desde el archivo .conf
QString friendswindow::loadAuthToken() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + "/Grace Hopper/Sota, Caballo y Rey.conf";
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo cargar el archivo de configuración.");
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
        QMessageBox::warning(this, "Error", "No se encontró el token en el archivo de configuración.");
    }
    return token;
}

// Función para obtener la lista de amigos
void friendswindow::fetchFriends() {
    QString token = loadAuthToken();
    if(token.isEmpty()) return;

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/usuarios/obtener_amigos/"));
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        // Comprobar si la petición respondió con un código 401 (no autorizado)
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            // Crear un diálogo modal similar al de confirmación de salir
            QDialog *expiredDialog = new QDialog(this);
            expiredDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
            // En lugar de un tamaño fijo, usamos adjustSize más adelante
            // expiredDialog->setFixedSize(300, 150);  <-- Se comenta

            expiredDialog->setStyleSheet(
                "QDialog {"
                "  background-color: #171718;"
                "  border-radius: 5px;"
                "  padding: 20px;"
                "}"
                );

            QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(expiredDialog);
            dialogShadow->setBlurRadius(10);
            dialogShadow->setColor(QColor(0, 0, 0, 80));
            dialogShadow->setOffset(4, 4);
            expiredDialog->setGraphicsEffect(dialogShadow);

            QVBoxLayout *dialogLayout = new QVBoxLayout(expiredDialog);
            QLabel *expiredLabel = new QLabel("Su sesión ha caducado, por favor, vuelva a iniciar sesión.", expiredDialog);

            // Activamos wordWrap para que salte de línea si no cabe
            expiredLabel->setWordWrap(true);
            // Ajustamos un estilo con fuente un poco más pequeña, por si hace falta
            expiredLabel->setStyleSheet("color: white; font-size: 16px;");
            expiredLabel->setAlignment(Qt::AlignCenter);
            dialogLayout->addWidget(expiredLabel);

            QPushButton *okButton = new QPushButton("OK", expiredDialog);
            okButton->setStyleSheet(
                "QPushButton {"
                "  background-color: #c2c2c3;"
                "  color: #171718;"
                "  border-radius: 15px;"
                "  font-size: 20px;"
                "  font-weight: bold;"
                "  padding: 12px 25px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #9b9b9b;"
                "}"
                );
            okButton->setFixedSize(100, 40);

            QHBoxLayout *btnLayout = new QHBoxLayout();
            btnLayout->addStretch();
            btnLayout->addWidget(okButton);
            btnLayout->addStretch();
            dialogLayout->addLayout(btnLayout);

            connect(okButton, &QPushButton::clicked, [=]() {
                expiredDialog->close();
                qApp->quit();
            });

            // Dejamos que se ajuste automáticamente al contenido
            expiredDialog->adjustSize();

            // Posicionar el diálogo centrado en la ventana actual
            expiredDialog->move(this->geometry().center() - expiredDialog->rect().center());
            expiredDialog->show();


            reply->deleteLater();
            return;
        }
        QByteArray response = reply->readAll();
        qDebug() << "fetchFriends response:" << response;
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            if(obj.contains("amigos")) {
                friendsListWidget->clear();
                QJsonArray amigos = obj["amigos"].toArray();
                for(const QJsonValue &value : amigos) {
                    QJsonObject amigo = value.toObject();

                    // Obtener el nombre usando la clave correcta
                    QString nombre = "Nombre no disponible";
                    if (amigo.contains("nombre"))
                        nombre = amigo["nombre"].toString();
                    else if (amigo.contains("Nombre"))
                        nombre = amigo["Nombre"].toString();
                    else if (amigo.contains("username"))
                        nombre = amigo["username"].toString();

                    QListWidgetItem *item = new QListWidgetItem(nombre, friendsListWidget);
                    // Convertir el id (entero) a QString
                    QString friendId = QString::number(amigo["id"].toInt());
                    item->setData(Qt::UserRole, friendId);
                }
            }
        }
        reply->deleteLater();
    });
}

// Función para obtener las solicitudes de amistad y mostrarlas en widgets personalizados.
void friendswindow::fetchRequests() {
    QString token = loadAuthToken();
    if(token.isEmpty()) return;

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/usuarios/listar_solicitudes_amistad/"));
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        // Comprobar si la petición respondió con un código 401 (no autorizado)
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            // Crear un diálogo modal similar al de confirmación de salir
            QDialog *expiredDialog = new QDialog(this);
            expiredDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
            // En lugar de un tamaño fijo, usamos adjustSize más adelante
            // expiredDialog->setFixedSize(300, 150);  <-- Se comenta

            expiredDialog->setStyleSheet(
                "QDialog {"
                "  background-color: #171718;"
                "  border-radius: 5px;"
                "  padding: 20px;"
                "}"
                );

            QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(expiredDialog);
            dialogShadow->setBlurRadius(10);
            dialogShadow->setColor(QColor(0, 0, 0, 80));
            dialogShadow->setOffset(4, 4);
            expiredDialog->setGraphicsEffect(dialogShadow);

            QVBoxLayout *dialogLayout = new QVBoxLayout(expiredDialog);
            QLabel *expiredLabel = new QLabel("Su sesión ha caducado, por favor, vuelva a iniciar sesión.", expiredDialog);

            // Activamos wordWrap para que salte de línea si no cabe
            expiredLabel->setWordWrap(true);
            // Ajustamos un estilo con fuente un poco más pequeña, por si hace falta
            expiredLabel->setStyleSheet("color: white; font-size: 16px;");
            expiredLabel->setAlignment(Qt::AlignCenter);
            dialogLayout->addWidget(expiredLabel);

            QPushButton *okButton = new QPushButton("OK", expiredDialog);
            okButton->setStyleSheet(
                "QPushButton {"
                "  background-color: #c2c2c3;"
                "  color: #171718;"
                "  border-radius: 15px;"
                "  font-size: 20px;"
                "  font-weight: bold;"
                "  padding: 12px 25px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #9b9b9b;"
                "}"
                );
            okButton->setFixedSize(100, 40);

            QHBoxLayout *btnLayout = new QHBoxLayout();
            btnLayout->addStretch();
            btnLayout->addWidget(okButton);
            btnLayout->addStretch();
            dialogLayout->addLayout(btnLayout);

            connect(okButton, &QPushButton::clicked, [=]() {
                expiredDialog->close();
                qApp->quit();
            });

            // Dejamos que se ajuste automáticamente al contenido
            expiredDialog->adjustSize();

            // Posicionar el diálogo centrado en la ventana actual
            expiredDialog->move(this->geometry().center() - expiredDialog->rect().center());
            expiredDialog->show();


            reply->deleteLater();
            return;
        }
        QByteArray response = reply->readAll();
        qDebug() << "fetchRequests response:" << response;
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            if(obj.contains("solicitudes")) {
                requestsListWidget->clear();
                QJsonArray solicitudes = obj["solicitudes"].toArray();
                for(const QJsonValue &value : solicitudes) {
                    QJsonObject solicitud = value.toObject();
                    QListWidgetItem *item = new QListWidgetItem(requestsListWidget);
                    QWidget *widget = createRequestWidget(solicitud);
                    // Asigna el sizeHint según el widget creado
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

    // Actualizamos la variable miembro con la búsqueda actual
    currentSearchQuery = searchText;

    // Si no hay texto, limpiamos la lista y salimos
    if (searchText.isEmpty()) {
        searchResultsListWidget->clear();
        return;
    }

    // Limpiamos la lista antes de hacer la petición
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
        // Comprobar si la petición respondió con un código 401 (no autorizado)
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            // Crear un diálogo modal similar al de confirmación de salir
            QDialog *expiredDialog = new QDialog(this);
            expiredDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
            // En lugar de un tamaño fijo, usamos adjustSize más adelante
            // expiredDialog->setFixedSize(300, 150);  <-- Se comenta

            expiredDialog->setStyleSheet(
                "QDialog {"
                "  background-color: #171718;"
                "  border-radius: 5px;"
                "  padding: 20px;"
                "}"
                );

            QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(expiredDialog);
            dialogShadow->setBlurRadius(10);
            dialogShadow->setColor(QColor(0, 0, 0, 80));
            dialogShadow->setOffset(4, 4);
            expiredDialog->setGraphicsEffect(dialogShadow);

            QVBoxLayout *dialogLayout = new QVBoxLayout(expiredDialog);
            QLabel *expiredLabel = new QLabel("Su sesión ha caducado, por favor, vuelva a iniciar sesión.", expiredDialog);

            // Activamos wordWrap para que salte de línea si no cabe
            expiredLabel->setWordWrap(true);
            // Ajustamos un estilo con fuente un poco más pequeña, por si hace falta
            expiredLabel->setStyleSheet("color: white; font-size: 16px;");
            expiredLabel->setAlignment(Qt::AlignCenter);
            dialogLayout->addWidget(expiredLabel);

            QPushButton *okButton = new QPushButton("OK", expiredDialog);
            okButton->setStyleSheet(
                "QPushButton {"
                "  background-color: #c2c2c3;"
                "  color: #171718;"
                "  border-radius: 15px;"
                "  font-size: 20px;"
                "  font-weight: bold;"
                "  padding: 12px 25px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #9b9b9b;"
                "}"
                );
            okButton->setFixedSize(100, 40);

            QHBoxLayout *btnLayout = new QHBoxLayout();
            btnLayout->addStretch();
            btnLayout->addWidget(okButton);
            btnLayout->addStretch();
            dialogLayout->addLayout(btnLayout);

            connect(okButton, &QPushButton::clicked, [=]() {
                expiredDialog->close();
                qApp->quit();
            });

            // Dejamos que se ajuste automáticamente al contenido
            expiredDialog->adjustSize();

            // Posicionar el diálogo centrado en la ventana actual
            expiredDialog->move(this->geometry().center() - expiredDialog->rect().center());
            expiredDialog->show();


            reply->deleteLater();
            return;
        }

        // Si la búsqueda actual ha cambiado desde que se lanzó esta consulta, ignoramos la respuesta.
        if (currentSearchQuery != queryText) {
            reply->deleteLater();
            return;
        }

        // Limpiamos la lista nuevamente para asegurarnos
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




// Crea un widget personalizado para cada usuario en los resultados de búsqueda
QWidget* friendswindow::createSearchResultWidget(const QJsonObject &usuario) {
    QWidget *widget = new QWidget();
    // Establece un tamaño mínimo para garantizar visibilidad
    widget->setMinimumSize(300, 130);

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10,2,10,2);
    layout->setSpacing(7);

    QVBoxLayout *infoLayout = new QVBoxLayout();

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
    nameLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    infoLayout->addWidget(nameLabel);

    int wins = usuario.contains("victorias") ? usuario["victorias"].toInt() : 0;
    int losses = usuario.contains("derrotas") ? usuario["derrotas"].toInt() : 0;
    double ratio = (wins + losses > 0) ? (wins * 100.0 / (wins + losses)) : 0.0;
    QLabel *statsLabel = new QLabel(
        QString("Victorias: %1   Derrotas: %2   Ratio: %3%")
            .arg(wins)
            .arg(losses)
            .arg(ratio, 0, 'f', 2),
        widget
        );
    statsLabel->setStyleSheet("color: white; font-size: 18px;");
    infoLayout->addWidget(statsLabel);

    layout->addLayout(infoLayout);
    layout->addStretch();

    QPushButton *addButton = new QPushButton("Agregar amigo", widget);
    addButton->setStyleSheet("background-color: #4CAF50; color: white; font-size: 18px; padding: 10px; border-radius: 10px;");

    // Convertir el id (entero) a QString
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

    widget->setLayout(layout);
    return widget;
}

// Crea un widget personalizado para cada solicitud de amistad
QWidget* friendswindow::createRequestWidget(const QJsonObject &solicitud) {
    QWidget *widget = new QWidget();
    // Establece un tamaño mínimo para el widget de solicitud
    widget->setMinimumSize(300, 70);

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(10);

    QLabel *nameLabel = new QLabel(solicitud["solicitante"].toString(), widget);
    nameLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    layout->addWidget(nameLabel);
    layout->addStretch();

    QPushButton *acceptBtn = new QPushButton("Aceptar", widget);
    acceptBtn->setStyleSheet("background-color: #4CAF50; color: white; font-size: 18px; padding: 10px; border-radius: 10px;");
    // Convertir el id de la solicitud (entero) a QString
    QString solicitudId = QString::number(solicitud["id"].toInt());
    acceptBtn->setProperty("solicitudId", solicitudId);
    connect(acceptBtn, &QPushButton::clicked, this, &friendswindow::acceptRequest);
    layout->addWidget(acceptBtn);

    QPushButton *rejectBtn = new QPushButton("Rechazar", widget);
    rejectBtn->setStyleSheet("background-color: #E53935; color: white; font-size: 18px; padding: 10px; border-radius: 10px;");
    rejectBtn->setProperty("solicitudId", solicitudId);
    connect(rejectBtn, &QPushButton::clicked, this, &friendswindow::rejectRequest);
    layout->addWidget(rejectBtn);

    widget->setLayout(layout);
    return widget;
}

// Envía solicitud de amistad al presionar "Agregar amigo"
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
    if(token.isEmpty()) {
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
        if(reply->error() == QNetworkReply::NoError) {
            qDebug() << "Solicitud enviada correctamente para userId:" << userId;
            button->setText("Solicitud enviada!");
            button->setEnabled(false);
            QMessageBox::information(this, "Solicitud enviada", "Se ha enviado la solicitud de amistad.");
        } else {
            qDebug() << "Error al enviar la solicitud para userId:" << userId << " Error:" << reply->errorString();
            QMessageBox::warning(this, "Error", "No se pudo enviar la solicitud.");
        }
        reply->deleteLater();
    });
}

// Acepta una solicitud de amistad
void friendswindow::acceptRequest() {
    QObject *senderObj = sender();
    if (!senderObj) return;
    QString solicitudId = senderObj->property("solicitudId").toString();
    if (solicitudId.isEmpty()) return;
    QString token = loadAuthToken();
    if(token.isEmpty()) return;

    QUrl url("http://188.165.76.134:8000/usuarios/aceptar_solicitud_amistad/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth", token.toUtf8());

    QJsonObject json;
    json["solicitud_id"] = solicitudId;
    QJsonDocument doc(json);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if(reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Solicitud aceptada", "Has aceptado la solicitud de amistad.");
            fetchRequests();
            fetchFriends();
        } else {
            QMessageBox::warning(this, "Error", "No se pudo aceptar la solicitud.");
        }
        reply->deleteLater();
    });
}

// Deniega una solicitud de amistad
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
            QMessageBox::information(this, "Solicitud denegada", "Has rechazado la solicitud de amistad.");
            fetchRequests();
        } else {
            QMessageBox::warning(this, "Error", "No se pudo rechazar la solicitud.");
        }
        reply->deleteLater();
    });
}
