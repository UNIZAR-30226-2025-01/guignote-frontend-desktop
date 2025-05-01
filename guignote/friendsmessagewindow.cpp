#include "friendsmessagewindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWebSocket>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

 FriendsMessageWindow::FriendsMessageWindow(const QString &userKey,
                                            const QString &friendId,
                                            const QString &friendName,
                                            QWidget *parent)
: QWidget(parent),
    friendID(friendId),
    usr(friendName)
{
    // Ventana sin borde y estilo
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    networkManager = new QNetworkAccessManager(this);




    ownID = loadOwnId(userKey);
    setupUI(userKey);
    loadMessages(userKey);
    setupWebSocketConnection(userKey);
}


void FriendsMessageWindow::setupUI(const QString &userKey)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20,20,20,20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // — Header —
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel(usr, this);
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(22,22));
    closeButton->setFixedSize(35,35);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 17px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // — Lista de mensajes —
    messagesListWidget = new QListWidget(this);
    messagesListWidget->setStyleSheet(
        "background-color: #292A2D; color: white; font-size: 16px; padding: 10px; "
        "border-radius: 10px; border: 1px solid #444;"
        );
    messagesListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    mainLayout->addWidget(messagesListWidget);

    // — Entrada + botón enviar —
    QHBoxLayout *inputLayout = new QHBoxLayout();
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

    sendButton = new QPushButton("Enviar", this);
    sendButton->setFixedHeight(40);
    sendButton->setStyleSheet(
        "QPushButton { background-color: #1D4536; color: #F9F9F4; font-size: 16px;"
        "padding: 8px 15px; border-radius: 10px; }"
        "QPushButton:hover { background-color: #2A5C45; }"
        );
    connect(sendButton, &QPushButton::clicked, this, [=]() {
        this->sendMessage(userKey);
    });

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);
}

void FriendsMessageWindow::setupWebSocketConnection(const QString &userKey)
{
    webSocket = new QWebSocket();
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qWarning() << "FriendsMessageWindow: Token vacío, no se conecta WebSocket.";
        return;
    }

    QString urlString = QString("ws://188.165.76.134:8000/ws/chat/%1/?token=%2")
                            .arg(friendID, token);
    webSocket->open(QUrl(urlString));

    connect(webSocket, &QWebSocket::connected,    this, &FriendsMessageWindow::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &FriendsMessageWindow::onDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived,
            this, &FriendsMessageWindow::onTextMessageReceived);
}

void FriendsMessageWindow::onConnected()
{
    qDebug() << "WebSocket conectado.";
}

void FriendsMessageWindow::onDisconnected()
{
    qDebug() << "WebSocket desconectado.";
}

void FriendsMessageWindow::onTextMessageReceived(const QString &rawMessage)
{
    // Parsear JSON
    QJsonDocument doc = QJsonDocument::fromJson(rawMessage.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    if (obj.contains("error")) {
        qWarning() << "Error WS:" << obj["error"].toString();
        return;
    }

    QString senderId = QString::number(obj["emisor"].toInt());
    QString content  = obj["contenido"].toString();

    if (senderId == ownID)
        return;

    appendMessage(senderId, content);
    emit newMessageReceived(senderId);
}

void FriendsMessageWindow::sendMessage(const QString &userKey)
{
    QString text = messageInput->text().trimmed();
    if (text.isEmpty()) {
        qDebug() << "No se envía mensaje vacío.";
        return;
    }

    // --- 1) Mostrar inmediatamente en la UI ---
    appendMessage(ownID, text);

    // --- 2) Enviar por WebSocket para entrega instantánea ---
    if (!webSocket->isValid())
        setupWebSocketConnection(userKey);

    QJsonObject wsJson{{"contenido", text}};
    webSocket->sendTextMessage(QString::fromUtf8(QJsonDocument(wsJson).toJson()));
    messageInput->clear();

    // --- 3) POST REST para guardar en el servidor ---
    QString token = loadAuthToken(userKey);
    QNetworkRequest postReq(QUrl("http://188.165.76.134:8000/mensajes/enviar_mensaje/"));
    postReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    postReq.setRawHeader("Auth", token.toUtf8());

    QJsonObject body;
    body["receptor_id"] = friendID;
    body["contenido"]    = text;

    QNetworkReply *postReply = networkManager->post(postReq, QJsonDocument(body).toJson());
    connect(postReply, &QNetworkReply::finished, this, [=]() {
        if (postReply->error() != QNetworkReply::NoError) {
            qWarning() << "Error al guardar mensaje:" << postReply->errorString();
        } else {
            // opcional: refrescar lista para asegurar consistencia
            loadMessages(userKey);
        }
        postReply->deleteLater();
    });
}


void FriendsMessageWindow::loadMessages(const QString &userKey)
{
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qWarning() << "No se pudo obtener token.";
        return;
    }

    QUrl url(QString("http://188.165.76.134:8000/mensajes/obtener_mensajes/?receptor_id=%1")
                 .arg(friendID));
    QNetworkRequest request(url);
    request.setRawHeader("Auth", token.toUtf8());

    auto *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray data = reply->readAll();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Error al cargar mensajes:" << reply->errorString();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return;
        QJsonArray arr = doc.object()["mensajes"].toArray();
        // Limpiar y volver a añadir
        messagesListWidget->clear();
        for (int i = arr.size()-1; i >= 0; --i) {
            auto obj = arr[i].toObject();
            QString senderId = QString::number(obj["emisor"].toInt());
            QString content  = obj["contenido"].toString();
            appendMessage(senderId, content);
        }
    });
}

void FriendsMessageWindow::appendMessage(const QString &senderId,
                                         const QString &content)
{
    QLabel *lbl = new QLabel(content);
    lbl->setWordWrap(true);
    lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QWidget *container = new QWidget();
    // <-- Aquí quitas cualquier borde/blanco
    container->setStyleSheet("background: transparent; border: none;");

    QVBoxLayout *lay = new QVBoxLayout(container);
    lay->setContentsMargins(5,5,5,5);
    lay->addWidget(lbl);

    if (senderId == ownID) {
        lbl->setStyleSheet(
            "background-color: white; color: black; padding: 8px; font-size: 16px;"
            "border-radius: 10px; max-width: 400px;"
            );
        lay->setAlignment(lbl, Qt::AlignRight);
    } else {
        lbl->setStyleSheet(
            "background-color: #1D4536; color: #F9F9F4; padding: 8px; font-size: 16px;"
            "border-radius: 10px; max-width: 400px;"
            );
        lay->setAlignment(lbl, Qt::AlignLeft);
    }

    auto *item = new QListWidgetItem();
    messagesListWidget->addItem(item);
    messagesListWidget->setItemWidget(item, container);

    QTimer::singleShot(0, this, [=]() {
        lbl->adjustSize();
        int h = lbl->height() * 0.35;
        if (h < 40) h = 40;
        item->setSizeHint(QSize(400, h + 10));
        messagesListWidget->scrollToBottom();
    });
}



QString FriendsMessageWindow::loadAuthToken(const QString &userKey)
{
    // Lee con QSettings en INI
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                   + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
    QSettings settings(path, QSettings::IniFormat);
    return settings.value("auth/token").toString();
}

QString FriendsMessageWindow::loadOwnId(const QString &userKey)
{
    // 1) Obtener el token
    QString token = loadAuthToken(userKey);
    if (token.isEmpty())
        return QString();

    // 2) Separar las tres partes del JWT
    const QStringList parts = token.split('.');
    if (parts.size() < 2)
        return QString();

    // 3) Decodificar la parte del payload (Base64URL → Base64)
    QByteArray payload = parts.at(1).toUtf8();
    payload.replace('-', '+');
    payload.replace('_', '/');
    switch (payload.size() % 4) {
    case 2: payload.append("=="); break;
    case 3: payload.append('=');  break;
    default: break;
    }

    // 4) Base64 → JSON
    QByteArray decoded = QByteArray::fromBase64(payload);
    QJsonDocument doc = QJsonDocument::fromJson(decoded);
    if (!doc.isObject())
        return QString();

    QJsonObject obj = doc.object();

    // 5) Extraer el campo correcto (ajusta el nombre si tu JWT usa otro claim)
    if (obj.contains("user_id"))
        return QString::number(obj.value("user_id").toInt());
    else if (obj.contains("id"))
        return QString::number(obj.value("id").toInt());

    return QString();
}


FriendsMessageWindow::~FriendsMessageWindow()
{
    webSocket->close();
    delete webSocket;
    qDebug() << "FriendsMessageWindow destruida";
}
