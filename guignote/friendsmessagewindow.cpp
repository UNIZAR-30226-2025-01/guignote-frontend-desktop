/**
 * @file friendsmessagewindow.cpp
 * @brief Implementación de la clase FriendsMessageWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de la clase FriendsMessageWindow, encargada de
 * gestionar la interfaz de chat entre usuarios, la conexión WebSocket y
 * el envío/recepción de mensajes.
 */

#include "friendsmessagewindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
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


/**
 * @brief Constructor de la clase FriendsMessageWindow.
 * @param userKey Clave identificadora del usuario.
 * @param friendId Identificador del amigo con quien se conversa.
 * @param friendName Nombre del amigo.
 * @param parent Widget padre.
 */
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

/**
 * @brief Crea y configura la interfaz de usuario.
 * @param userKey Clave del usuario para gestionar eventos de envío.
 */
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

/**
 * @brief Establece la conexión WebSocket y sus callbacks.
 * @param userKey Clave del usuario para obtener token de autenticación.
 */
void FriendsMessageWindow::setupWebSocketConnection(const QString &userKey)
{
    webSocket = new QWebSocket();
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qWarning() << "FriendsMessageWindow: Token vacío, no se conecta WebSocket.";
        return;
    }

    QUrl url;
    url.setScheme("ws");
    url.setHost("188.165.76.134");
    url.setPort(8000);
    url.setPath(QString("/ws/chat/%1/").arg(friendID));
    url.setQuery(QString("token=%1").arg(token));

    webSocket->open(url);

    connect(webSocket, &QWebSocket::connected,    this, &FriendsMessageWindow::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &FriendsMessageWindow::onDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived,
            this, &FriendsMessageWindow::onTextMessageReceived);
}

/**
 * @brief Callback cuando el WebSocket se conecta correctamente.
 */
void FriendsMessageWindow::onConnected()
{
    qDebug() << "WebSocket conectado.";
}

/**
 * @brief Callback cuando el WebSocket se desconecta.
 */
void FriendsMessageWindow::onDisconnected()
{
    qDebug() << "WebSocket desconectado.";
}

/**
 * @brief Maneja la recepción de un nuevo mensaje vía WebSocket.
 * @param rawMessage Cadena JSON del mensaje recibido.
 */
void FriendsMessageWindow::onTextMessageReceived(const QString &rawMessage)
{
    // 1) Parsear el JSON recibido
    QJsonDocument doc = QJsonDocument::fromJson(rawMessage.toUtf8());
    if (!doc.isObject())                 // no era un objeto JSON → descarta
        return;

    QJsonObject obj = doc.object();
    if (obj.contains("error")) {         // el servidor manda un error
        qWarning() << "Error WS:" << obj["error"].toString();
        return;
    }

    // 2) Extraer los campos que ya tenías
    QString senderId = QString::number(obj["emisor"].toInt());
    QString content  = obj["contenido"].toString();
    QString ts       = obj["fecha_envio"].toString();   // AAAA-MM-DD HH:MM:SS

    // (opcional) no muestres tu propio eco
    if (senderId == ownID)
        return;

    // 3) Construir la clave única y filtrar duplicados
    QString key = ts + "|" + senderId + "|" + content;  // ← clave compuesta
    if (m_shownKeys.contains(key))
        return;                         // ya se mostró → ignora

    m_shownKeys.insert(key);            // marca como nuevo

    // 4) Mostrar el mensaje y disparar la señal
    appendMessage(senderId, content);
    emit newMessageReceived(senderId);
}

/**
 * @brief Envía un mensaje tanto por WebSocket como por REST.
 * @param userKey Clave de usuario para token y conexión.
 */
void FriendsMessageWindow::sendMessage(const QString &userKey)
{
    const QString textoAEnviar = messageInput->text().trimmed();
    if (textoAEnviar.isEmpty()) return;

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


/**
 * @brief Descarga y muestra el historial de mensajes.
 * @param userKey Clave del usuario para autenticación REST.
 */
void FriendsMessageWindow::loadMessages(const QString &userKey)
{
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) return;

    struct Msg {
        QDateTime ts;
        QString   sender;
        QString   text;
        QString   key;        //  "<ts>|<sender>|<text>"
    };
    QVector<Msg> mensajes;

    /* --- 1. Descarga REST ------------------------------------------------ */
    QUrl url(QString("http://188.165.76.134:8000/mensajes/obtener/?receptor_id=%1")
                 .arg(friendID));
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
            auto  o   = v.toObject();
            QString tsStr   = o["fecha_envio"].toString();            // "AAAA-MM-DD HH:MM:SS"
            QDateTime ts    = QDateTime::fromString(tsStr,
                                                 "yyyy-MM-dd HH:mm:ss");
            QString sender  = QString::number(o["emisor"].toInt());
            QString text    = o["contenido"].toString();
            QString key     = tsStr + "|" + sender + "|" + text;

            mensajes.append({ts, sender, text, key});
        }
    }
    r->deleteLater();

    /* --- 2. Ordenar por fecha ------------------------------------------- */
    std::sort(mensajes.begin(), mensajes.end(),
              [](const Msg &a, const Msg &b){ return a.ts < b.ts; });

    /* --- 3. Poblar la UI, filtrando duplicados -------------------------- */
    messagesListWidget->clear();          // borramos la lista actual

    for (const Msg &m : mensajes)
    {
        if (m_shownKeys.contains(m.key))
            continue;                     // ya estaba (raro, pero por si acaso)

        m_shownKeys.insert(m.key);
        appendMessage(m.sender, m.text);  // tu burbuja
    }

    messagesListWidget->scrollToBottom();
}

/**
 * @brief Añade un mensaje a la interfaz.
 * @param senderId ID del remitente del mensaje.
 * @param content Contenido textual del mensaje.
 */
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
        const int bubbleMaxWidth = 500;   // sube o baja a tu gusto

        // ①  Estilo común para ambas burbujas
        QString common = QString(
                             "padding: 10px; font-size: 16px; border-radius: 10px; "
                             "max-width: %1px;").arg(bubbleMaxWidth);

        // ②  Color según quién envía
        if (senderId == ownID) {
            lbl->setStyleSheet("background-color: white; color: black; " + common);
            lay->setAlignment(lbl, Qt::AlignRight);
        } else {
            lbl->setStyleSheet("background-color: #1D4536; color: #F9F9F4; " + common);
            lay->setAlignment(lbl, Qt::AlignLeft);
        }

        // ③  Calcular tamaño óptimo sin factor 0.35
        lbl->adjustSize();
        int h = lbl->sizeHint().height();                       // alto real
        int listW   = messagesListWidget->viewport()->width();  // ancho disponible
        int targetW = qMin(bubbleMaxWidth, listW - 40);         // deja 20 px de margen a cada lado

        item->setSizeHint(QSize(targetW, h + 10));
        messagesListWidget->scrollToBottom();
    });
}


/**
 * @brief Obtiene el token de autenticación almacenado localmente.
 * @param userKey Clave del usuario.
 * @return Token JWT como cadena.
 */
QString FriendsMessageWindow::loadAuthToken(const QString &userKey)
{
    QString appName = QString("Sota, Caballo y Rey_%1").arg(userKey);
    QSettings settings("Grace Hopper", appName);
    return settings.value("auth/token").toString();
}

/**
 * @brief Extrae el ID propio del usuario desde el token JWT.
 * @param userKey Clave del usuario.
 * @return ID del usuario como cadena.
 */
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

/**
 * @brief Destructor de FriendsMessageWindow. Libera recursos.
 */
FriendsMessageWindow::~FriendsMessageWindow()
{
    webSocket->close();
    delete webSocket;
    qDebug() << "FriendsMessageWindow destruida";
}
