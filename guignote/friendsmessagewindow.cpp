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
    loadMessages();
    setupWebSocketConnection();
}

void FriendsMessageWindow::setupWebSocketConnection()
{
    webSocket = new QWebSocket();

    QString urlString = QString("ws://188.165.76.134:8000/ws/chat_partida/%1/?token=%2")
                            .arg(friendID)
                            .arg(loadAuthToken());
    QUrl url(urlString);

    webSocket->open(url);

    connect(webSocket, &QWebSocket::connected, this, &FriendsMessageWindow::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &FriendsMessageWindow::onDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived,
            this, &FriendsMessageWindow::onTextMessageReceived);
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

void FriendsMessageWindow::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "Mensaje recibido inválido.";
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains("error")) {
        qDebug() << "Error recibido:" << obj["error"].toString();
        return;
    }

    QString contenido = obj["contenido"].toString();
    QJsonObject emisor = obj["emisor"].toObject();
    int emisorId = emisor["id"].toInt();

    // Crear y añadir el mensaje directamente a la lista
    addMessageToList(contenido, emisorId);
}

void FriendsMessageWindow::addMessageToList(const QString &contenido, int emisorId)
{
    QLabel *messageLabel = new QLabel(contenido);
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    messageLabel->setAlignment(Qt::AlignLeft);

    QWidget *messageContainer = new QWidget();
    QVBoxLayout *containerLayout = new QVBoxLayout(messageContainer);
    containerLayout->setContentsMargins(5, 5, 5, 5);
    containerLayout->addWidget(messageLabel);

    QListWidgetItem *item = new QListWidgetItem();
    messagesListWidget->addItem(item);
    messagesListWidget->setItemWidget(item, messageContainer);
    messagesListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    QTimer::singleShot(0, this, [=]() {
        adjustMessageSize(item, messageLabel);
        messagesListWidget->scrollToBottom();
    });

    if (QString::number(emisorId) == usr) {
        messageLabel->setStyleSheet(
            "background-color: #2196F3; color: white; padding: 8px; font-size: 16px;"
            "border-radius: 10px; min-width: 100px; max-width: 400px;"
            );
        containerLayout->setAlignment(Qt::AlignRight);
    } else {
        messageLabel->setStyleSheet(
            "background-color: white; color: black; padding: 8px; font-size: 16px;"
            "border-radius: 10px; min-width: 100px; max-width: 400px;"
            );
        containerLayout->setAlignment(Qt::AlignLeft);
    }
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
    QString message = messageInput->text().trimmed();
    messageInput->clear();

    if (message.isEmpty()) {
        qDebug() << "Intento de enviar mensaje vacío.";
        return;
    }

    if (!webSocket->isValid()) {
        setupWebSocketConnection();
    }

    QJsonObject json;
    json["contenido"] = message;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    webSocket->sendTextMessage(QString::fromUtf8(jsonData));

    qDebug() << "Mensaje enviado:" << message;
}



void FriendsMessageWindow::loadMessages()
{
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "Token inválido.";
        return;
    }

    QUrl url(QString("http://188.165.76.134:8000/chat_partida/obtener/?chat_id=%1").arg(friendID));
    QNetworkRequest request(url);
    request.setRawHeader("Auth", token.toUtf8());

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QByteArray responseData = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(responseData);

            if (!doc.isObject()) {
                qDebug() << "JSON no válido.";
                return;
            }

            QJsonObject jsonResponse = doc.object();
            if (!jsonResponse.contains("mensajes")) {
                qDebug() << "No hay mensajes.";
                return;
            }

            QJsonArray messagesArray = jsonResponse["mensajes"].toArray();
            messagesListWidget->clear();

            for (int i = messagesArray.size() - 1; i >= 0; --i) {
                QJsonObject msgObj = messagesArray[i].toObject();
                QString contenido = msgObj["contenido"].toString();
                int senderId = msgObj["emisor"].toInt();

                addMessageToList(contenido, senderId);
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

FriendsMessageWindow::~FriendsMessageWindow()
{
    qDebug() << "Pantalla Cerrada";
    delete webSocket;
}

