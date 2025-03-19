#include "friendsmessagewindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QTimer>

FriendsMessageWindow::FriendsMessageWindow(QWidget *parent, QString ID, QString Usuario)
    : QWidget(parent)
{
    friendID = ID;
    usr = Usuario;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    networkManager = new QNetworkAccessManager(this);

    setupUI();
    connectWebSocket();
    loadMessages();

}

void FriendsMessageWindow::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // Encabezado
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel(usr, this);
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
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // **Lista donde se mostrarán los mensajes**
    messagesListWidget = new QListWidget(this);
    messagesListWidget->setStyleSheet(
        "background-color: #292A2D; color: white; font-size: 16px; padding: 10px; "
        "border-radius: 10px; border: 1px solid #444;"
        );
    mainLayout->addWidget(messagesListWidget);

    // Entrada de mensajes
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Escribe un mensaje...");
    messageInput->setStyleSheet(
        "background-color: #292A2D; color: white; font-size: 16px; padding: 10px; "
        "border-radius: 10px; border: 1px solid #444;"
        );
    messageInput->setFixedHeight(40);
    connect(messageInput, &QLineEdit::returnPressed, this, &FriendsMessageWindow::sendMessage);

    mainLayout->addWidget(messageInput);
}


void FriendsMessageWindow::sendMessage()
{
    // Obtener el texto ingresado en el campo de entrada
    QString message = messageInput->text().trimmed();
    messageInput->clear();

    // Verificar que el mensaje no esté vacío
    if (message.isEmpty()) {
        qDebug() << "Intento de enviar mensaje vacío.";
        return;
    }

    // Cargar el token de autenticación desde el archivo de configuración
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "No se pudo obtener el token de autenticación.";
        return;
    }

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/mensajes/enviar/"));
    request.setRawHeader("Auth", token.toUtf8());

    QJsonObject json;
    json["receptor_id"] = friendID;
    json["contenido"] = message;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    QNetworkReply *reply = networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        // Obtener el código de estado HTTP
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        if (!statusCode.isValid()) {
            qDebug() << "Error: No se pudo obtener el código de estado HTTP.";
            reply->deleteLater();
            return;
        }

        int httpStatus = statusCode.toInt();
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();

        // Manejo de respuestas según el código de estado HTTP
        switch (httpStatus) {
        case 201:
            loadMessages();
            break;
        case 400:
            qDebug() << "❌ Error 400: Faltan campos o datos inválidos.";
            break;
        case 401:
            qDebug() << "❌ Error 401: Token inválido o expirado.";
            break;
        case 403:
            qDebug() << "❌ Error 403: Solo puedes chatear con amigos.";
            break;
        case 404:
            qDebug() << "❌ Error 404: Usuario receptor no encontrado.";
            break;
        case 405:
            qDebug() << "❌ Error 405: Método no permitido.";
            break;
        default:
            qDebug() << "⚠️ Error inesperado, código:" << httpStatus;
            qDebug() << "Respuesta del servidor:" << responseData;
            break;
        }

        reply->deleteLater();
    });
}

void FriendsMessageWindow::connectWebSocket() {
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "No se pudo obtener el token de autenticación.";
        return;
    }

    // Construir la URL con el token
    QString wsUrl = QString("ws://188.165.76.134:8000/ws/chat/%1/?token=%2")
                        .arg(friendID)
                        .arg(token);

    // Crear WebSocket
    socket = new QWebSocket();

    // Conectar señales del WebSocket
    connect(socket, &QWebSocket::connected, this, []() {
        qDebug() << "🔗 Conectado al WebSocket del chat.";
    });

    connect(socket, &QWebSocket::disconnected, this, []() {
        qDebug() << "❌ WebSocket desconectado.";
    });

    connect(socket, &QWebSocket::textMessageReceived, this, [this](const QString &message) {
        qDebug() << "📩 Mensaje recibido:" << message;

        // Convertir el mensaje a JSON
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) {
            qDebug() << "⚠️ Error: Respuesta del servidor no es un JSON válido.";
            return;
        }

        QJsonObject jsonObj = doc.object();
        if (jsonObj.contains("error")) {
            qDebug() << "⚠️ Error del WebSocket:" << jsonObj["error"].toString();
            return;
        }

        // Si recibimos un mensaje válido, recargamos la lista de mensajes
        loadMessages();
    });

    // Conectar WebSocket al servidor
    socket->open(QUrl(wsUrl));
}


void FriendsMessageWindow::loadMessages()
{
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "No se pudo obtener el token de autenticación.";
        return;
    }

    QUrl url(QString("http://188.165.76.134:8000/mensajes/obtener/?receptor_id=%1").arg(friendID));
    QNetworkRequest request(url);
    request.setRawHeader("Auth", token.toUtf8());

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QByteArray responseData = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(responseData);

            if (!doc.isObject()) {
                qDebug() << "Error: Respuesta del servidor no es un JSON válido.";
                return;
            }

            QJsonObject jsonResponse = doc.object();
            if (!jsonResponse.contains("mensajes")) {
                qDebug() << "No se encontraron mensajes en la respuesta.";
                return;
            }

            QJsonArray messagesArray = jsonResponse["mensajes"].toArray();
            messagesListWidget->clear();

            for (int i = messagesArray.size() - 1; i >= 0; --i) {
                if (!messagesArray[i].isObject()) continue;

                QJsonObject messageObject = messagesArray[i].toObject();
                QString senderId = QString::number(messageObject["emisor"].toInt());
                QString content = messageObject["contenido"].toString();

                QLabel *messageLabel = new QLabel(content);
                messageLabel->setWordWrap(true);
                messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                messageLabel->setAlignment(Qt::AlignLeft);

                // Crear un contenedor para el mensaje
                QWidget *messageContainer = new QWidget();
                messageContainer->setStyleSheet("border: none; background: transparent;");

                // Agregar un layout al contenedor del mensaje
                QVBoxLayout *containerLayout = new QVBoxLayout(messageContainer);
                containerLayout->setContentsMargins(5, 5, 5, 5);
                containerLayout->setSpacing(5);
                containerLayout->addWidget(messageLabel);

                // Crear y agregar el item a la lista
                QListWidgetItem *item = new QListWidgetItem();
                messagesListWidget->addItem(item);
                messagesListWidget->setItemWidget(item, messageContainer);

                // Ajustar la altura dinámicamente después del renderizado
                QTimer::singleShot(0, this, [=]() {
                    adjustMessageSize(item, messageLabel);
                });

                // Ajustar color y estilo de la burbuja según el emisor
                if (senderId != friendID) {
                    messageLabel->setStyleSheet(
                        "background-color: #2196F3; color: white; padding: 8px; font-size: 16px;"
                        "border-radius: 10px; min-width: 100px; min-height: 30px; max-width: 400px;"
                        );
                    containerLayout->setAlignment(Qt::AlignRight);
                } else {
                    messageLabel->setStyleSheet(
                        "background-color: white; color: black; padding: 8px; font-size: 16px;"
                        "border-radius: 10px; min-width: 100px; min-height: 30px; max-width: 400px;"
                        );
                    containerLayout->setAlignment(Qt::AlignLeft);
                }

                messagesListWidget->addItem(item);
                messagesListWidget->setItemWidget(item, messageContainer);
                messagesListWidget->scrollToBottom();
            }
        } else {
            qDebug() << "Error al cargar mensajes:" << reply->errorString();
        }

        reply->deleteLater();
    });
}

void FriendsMessageWindow::adjustMessageSize(QListWidgetItem *item, QLabel *messageLabel) {
    messageLabel->adjustSize();
    int labelHeight = (messageLabel->height())*0.35;
    if (labelHeight < 40) labelHeight = 40; // Asegurar un tamaño mínimo

    item->setSizeHint(QSize(400, labelHeight + 10)); // Ajustar tamaño correctamente
}


// Función para extraer el token de autenticación desde el archivo .conf
QString FriendsMessageWindow::loadAuthToken() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + "/Grace Hopper/Sota, Caballo y Rey.conf";
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo cargar el archivo de configuración.";
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
        qDebug() << "No se encontró el token en el archivo de configuración.";
    }
    return token;
}

FriendsMessageWindow::~FriendsMessageWindow() {
    if (socket) {
        socket->close();
        socket->deleteLater();
    }
}
