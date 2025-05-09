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
#include <QEventLoop>
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
    const QString textoAEnviar = messageInput->text().trimmed();
    if (textoAEnviar.isEmpty()) return;

    // 1) Guardar localmente ANTES de todo
    QString appName = QString("Sota, Caballo y Rey_%1").arg(userKey);
    QSettings cache("Grace Hopper", appName);
    cache.beginGroup("chat");
    cache.beginGroup(friendID);

    QStringList sentList = cache.value("sent").toStringList();
    QString entry = QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
                    + "|" + textoAEnviar;
    sentList << entry;
    if (sentList.size() > 200)
        sentList = sentList.mid(sentList.size() - 200);
    cache.setValue("sent", sentList);

    cache.endGroup();
    cache.endGroup();
    cache.sync();
    qDebug() << "[QSETTINGS] Mensajes enviados:" << sentList;

    // 2) Mostrar en UI
    appendMessage(ownID, textoAEnviar);
    messageInput->clear();

    // 3) Enviar por WS
    // 3) Enviar por WS
    if (!webSocket->isValid())
        setupWebSocketConnection(userKey);

    // Construye explícitamente un objeto JSON
    QJsonObject wsJson;
    wsJson["contenido"] = textoAEnviar;

    // Ahora ya no hay ambigüedad: pasas un QJsonObject
    QJsonDocument wsDoc(wsJson);
    webSocket->sendTextMessage(wsDoc.toJson());


    // 4) POST REST
    QString token = loadAuthToken(userKey);
    QNetworkRequest postReq(QUrl("http://188.165.76.134:8000/mensajes/enviar/"));
    postReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    postReq.setRawHeader("Auth", token.toUtf8());
    QJsonObject body{{"receptor_id", friendID},
                     {"contenido", textoAEnviar}};
    auto *postReply = networkManager->post(postReq, QJsonDocument(body).toJson());
    connect(postReply, &QNetworkReply::finished, this, [=]() {
        if (postReply->error() != QNetworkReply::NoError)
            qWarning() << "Error al guardar mensaje:" << postReply->errorString();
        postReply->deleteLater();
    });
}



void FriendsMessageWindow::loadMessages(const QString &userKey)
{
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) return;

    struct Msg { QDateTime ts; QString emisor, txt; };
    QVector<Msg> todos;

    // 2.1) Traer del servidor solo lo que te han enviado a ti
    {
        QUrl url(QString("http://188.165.76.134:8000/mensajes/obtener_mensajes/?receptor_id=%1")
                     .arg(ownID));
        QNetworkRequest req(url);
        req.setRawHeader("Auth", token.toUtf8());
        QNetworkReply *r = networkManager->get(req);

        QEventLoop loop;
        connect(r, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (r->error() == QNetworkReply::NoError) {
            QJsonArray arr = QJsonDocument::fromJson(r->readAll())
            .object()["mensajes"].toArray();
            for (auto v : arr) {
                auto o = v.toObject();
                Msg m;
                m.emisor = QString::number(o["emisor"].toInt());
                m.txt    = o["contenido"].toString();
                m.ts     = QDateTime::fromString(
                    o["fecha_envio"].toString(),
                    "yyyy-MM-dd HH:mm:ss");
                todos.append(m);
            }
        }
        r->deleteLater();
    }

    // 2.2) Añadir tus envíos locales desde QSettings
    {
        QString appName = QString("Sota, Caballo y Rey_%1").arg(userKey);
        QSettings cache("Grace Hopper", appName);
        cache.beginGroup("chat");
        cache.beginGroup(friendID);

        QStringList sentList = cache.value("sent").toStringList();

        cache.endGroup();   // sale de friendID
        cache.endGroup();   // sale de chat

        for (const QString &entry : sentList) {
            QStringList parts = entry.split('|', Qt::KeepEmptyParts);
            if (parts.size() != 2) continue;
            Msg m;
            m.ts     = QDateTime::fromString(parts[0], Qt::ISODate);
            m.emisor = ownID;
            m.txt    = parts[1];
            todos.append(m);
        }
    }

    // 3) Orden cronológico
    std::sort(todos.begin(), todos.end(),
              [](auto &a, auto &b){ return a.ts < b.ts; });

    // 4) Repoblar la UI
    messagesListWidget->clear();
    for (auto &m : todos)
        appendMessage(m.emisor, m.txt);
    messagesListWidget->scrollToBottom();
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
    QString appName = QString("Sota, Caballo y Rey_%1").arg(userKey);
    QSettings settings("Grace Hopper", appName);
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
