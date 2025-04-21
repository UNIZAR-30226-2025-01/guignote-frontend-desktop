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
#include <QWebSocket>

FriendsMessageWindow::FriendsMessageWindow(const QString &userKey,QWidget *parent,  QString ID, QString Usuario)
    : QWidget(parent)
{
    friendID = ID;
    usr = Usuario;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    networkManager = new QNetworkAccessManager(this);

    setupUI(userKey);
    loadMessages(userKey);
    setupWebSocketConnection(userKey);
}

void FriendsMessageWindow::setupWebSocketConnection(const QString &userKey)
{
    // Crear el QWebSocket
    webSocket = new QWebSocket();

    // Construir la URL con los parámetros necesarios
    QString urlString = QString("ws://188.165.76.134:8000/ws/chat/%1/?token=%2")
                            .arg(friendID)
                            .arg(loadAuthToken(userKey));
    QUrl url(urlString);

    // Abrir la conexión WebSocket
    webSocket->open(url);

    // Conectar señales a sus respectivos slots
    connect(webSocket, &QWebSocket::connected, this, &FriendsMessageWindow::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &FriendsMessageWindow::onDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived,
            this, [=](const QString &message) {
                this->onTextMessageReceived(message, userKey);
            });
}


void FriendsMessageWindow::onConnected()
{
    qDebug() << "Conexión WebSocket establecida.";
    // Aquí puedes enviar un mensaje de saludo o realizar otras acciones necesarias
}

void FriendsMessageWindow::onDisconnected()
{
    qDebug() << "Conexión WebSocket cerrada.";
    // Maneja la desconexión según sea necesario
}

void FriendsMessageWindow::onTextMessageReceived(const QString &message, const QString &userKey)
{
    Q_UNUSED(message);
    loadMessages(userKey); // Actualiza la lista de mensajes
}

void FriendsMessageWindow::setupUI(const QString &userKey)
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
    connect(messageInput, &QLineEdit::returnPressed, this, [=]() {
        this->sendMessage(userKey);
    });



    mainLayout->addWidget(messageInput);
}


void FriendsMessageWindow::sendMessage(const QString &userKey)
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
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qDebug() << "No se pudo obtener el token de autenticación.";
        return;
    }

    // Crear la URL del WebSocket con el receptor ID y el token
    QString urlString = QString("ws://188.165.76.134:8000/ws/chat/%1/?token=%2")
                            .arg(friendID)
                            .arg(token);
    QUrl url(urlString);

    // Si no hay conexión activa, establecer la conexión WebSocket
    if (!webSocket->isValid()) {
        webSocket->open(url);
    }

    // Crear el mensaje en formato JSON
    QJsonObject json;
    json["contenido"] = message;

    // Convertir el mensaje a QByteArray
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    // Enviar el mensaje por el WebSocket
    webSocket->sendTextMessage(QString::fromUtf8(jsonData));

    qDebug() << "Mensaje enviado:" << message;
}


void FriendsMessageWindow::loadMessages(const QString &userKey)
{
    QString token = loadAuthToken(userKey);
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
                messagesListWidget->setSelectionMode(QAbstractItemView::NoSelection);

                // Ajustar la altura dinámicamente después del renderizado
                QTimer::singleShot(0, this, [=]() {
                    adjustMessageSize(item, messageLabel);
                    messagesListWidget->scrollToBottom();
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
QString FriendsMessageWindow::loadAuthToken(const QString &userKey) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
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

FriendsMessageWindow::~FriendsMessageWindow()
{
    qDebug() << "Pantalla Cerrada";
    delete webSocket;
}

