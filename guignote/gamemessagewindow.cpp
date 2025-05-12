/**
 * @file GameMessageWindow.cpp
 * @brief Implementación de la clase GameMessageWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de la ventana de chat de partida, que gestiona
 * la interfaz de mensajería, la conexión WebSocket/Socket TCP y el
 * almacenamiento de histórico de mensajes.
 */


#include "gamemessagewindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>
#include <qjsonarray.h>
#include <QSettings>
#include <QStandardPaths>


QMap<QString, QList<QPair<QString,QString>>> GameMessageWindow::chatHistories;

/**
 * @brief Constructor de la ventana de chat de partida.
 * @param userKey Clave del usuario para autenticación.
 * @param parent Widget padre.
 * @param gameID Identificador de la partida/chat.
 * @param userID Identificador del usuario actual.
 */

GameMessageWindow::GameMessageWindow(const QString &userKey, QWidget *parent, const QString &gameID, const QString &userID)
    : QWidget(parent), chatID(gameID), userID(userID)
{
    // Ventana sin borde y estilo
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_DeleteOnClose);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    setupUI(userKey);
    setupWebSocketConnection(userKey);

    //  — Cargar historial previo si existiera —
    networkManager = new QNetworkAccessManager(this);
    loadChatHistoryFromServer(userKey);
}


/**
 * @brief Descarga el historial de chat de la partida mediante socket TCP.
 * @param userKey Clave del usuario para enviar en la petición.
 */

void GameMessageWindow::loadChatHistoryFromServer(const QString &userKey) {
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qDebug() << "GameMessageWindow: No se puede cargar historial, token vacío.";
        return;
    }

    QTcpSocket *socket = new QTcpSocket(this);
    socket->connectToHost("188.165.76.134", 8000);
    if (!socket->waitForConnected(3000)) {
        qDebug() << "GameMessageWindow: Error al conectar socket:" << socket->errorString();
        socket->deleteLater();
        return;
    }

    // Construimos la petición HTTP “a mano”
    QString httpReq = QString(
                          "GET /chat_partida/obtener/?chat_id=%1 HTTP/1.1\r\n"
                          "Host: 188.165.76.134:8000\r\n"
                          "Auth: %2\r\n"
                          "Connection: close\r\n"
                          "\r\n"
                          ).arg(chatID, token.trimmed());

    socket->write(httpReq.toUtf8());
    if (!socket->waitForBytesWritten(3000)) {
        qDebug() << "GameMessageWindow: Error al enviar petición:" << socket->errorString();
    }
    if (!socket->waitForReadyRead(5000)) {
        qDebug() << "GameMessageWindow: Timeout esperando respuesta.";
        socket->close();
        socket->deleteLater();
        return;
    }

    QByteArray raw = socket->readAll();
    socket->close();
    socket->deleteLater();

    // Separa cabeceras de cuerpo
    int idx = raw.indexOf("\r\n\r\n");
    if (idx < 0) {
        qDebug() << "GameMessageWindow: Respuesta HTTP malformada.";
        return;
    }
    QByteArray body = raw.mid(idx + 4);

    // Parseamos JSON y mostramos como antes
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qDebug() << "Respuesta de historial no es JSON válido.";
        qDebug() << "Raw response:" << QString(raw);
        return;
    }
    QJsonArray mensajes = doc.object()["mensajes"].toArray();
    for (int i = mensajes.size() - 1; i >= 0; --i) {
        QJsonObject msg = mensajes[i].toObject();
        QString senderId = QString::number(msg["emisor"].toInt());
        QString content  = msg["contenido"].toString();
        appendMessage(senderId, content);
    }
}


/**
 * @brief Destructor: cierra el WebSocket y libera recursos.
 */

GameMessageWindow::~GameMessageWindow() {
    qDebug() << "GameMessageWindow: Destructor, cerrando WebSocket.";
    webSocket->close();
    delete webSocket;
}

/**
 * @brief Configura la interfaz gráfica de la ventana de chat.
 * @param userKey Clave del usuario (se pasa para conectar señales).
 */

void GameMessageWindow::setupUI(const QString userKey) {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20,20,20,20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // — Header —
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Chat de Partida", this);
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(22,22));
    closeButton->setFixedSize(35,35);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 17px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

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
 * @brief Establece la conexión WebSocket para recibir mensajes en tiempo real.
 * @param userKey Clave del usuario usada para obtener el token.
 */

void GameMessageWindow::setupWebSocketConnection(const QString &userKey) {
    webSocket = new QWebSocket();
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qDebug() << "GameMessageWindow: Token vacío, no se puede conectar WebSocket.";
        return;
    }

    QString urlString = QString("ws://188.165.76.134:8000/ws/chat_partida/%1/?token=%2")
                            .arg(chatID, token);

    qDebug() << "GameMessageWindow: Conectando a" << urlString;
    webSocket->open(QUrl(urlString));

    connect(webSocket, &QWebSocket::connected, this, &GameMessageWindow::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &GameMessageWindow::onDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived,
            this, &GameMessageWindow::onTextMessageReceived);
}

/**
 * @brief Callback que se ejecuta cuando el WebSocket se conecta.
 */

void GameMessageWindow::onConnected() {
    qDebug() << "GameMessageWindow: WebSocket conectado.";
}

/**
 * @brief Callback que se ejecuta cuando el WebSocket se desconecta.
 */

void GameMessageWindow::onDisconnected() {
    qDebug() << "GameMessageWindow: WebSocket desconectado.";
}

/**
 * @brief Procesa un mensaje recibido por WebSocket.
 * @param message Cadena JSON con el mensaje.
 */

void GameMessageWindow::onTextMessageReceived(const QString &message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    if (obj.contains("error")) return;

    QJsonObject emisorObj = obj["emisor"].toObject();
    QString senderId = QString::number(emisorObj["id"].toInt());

    // IGNORAR SI ES MI PROPIO MENSAJE
    if (senderId == userID) return;

    QString content = obj["contenido"].toString();
    appendMessage(senderId, content);
}


/**
 * @brief Envía un mensaje: lo muestra en UI y lo manda por WebSocket.
 * @param userKey Clave del usuario para (re)conexión si hace falta.
 */

void GameMessageWindow::sendMessage(const QString &userKey) {
    // 1) Recuperar y validar el texto
    QString text = messageInput->text().trimmed();
    if (text.isEmpty()) {
        qDebug() << "GameMessageWindow: No se envía mensaje vacío.";
        return;
    }

    // 2) Mostrar inmediatamente tu burbuja (estilo “propio”)
    appendMessage(userID, text);

    // 3) Enviar el mensaje por WebSocket
    if (!webSocket->isValid()) {
        qDebug() << "GameMessageWindow: WebSocket no válido, reintentando conexión...";
        setupWebSocketConnection(userKey);
    }
    QJsonObject json;
    json["contenido"] = text;
    QJsonDocument doc(json);
    webSocket->sendTextMessage(QString::fromUtf8(doc.toJson()));
    qDebug() << "GameMessageWindow: Mensaje enviado:" << text;

    // 4) Limpiar el campo de entrada
    messageInput->clear();
}


/**
 * @brief Añade un mensaje al listado de chat con estilo de burbuja.
 * @param senderId ID del remitente.
 * @param content Texto del mensaje.
 */

void GameMessageWindow::appendMessage(const QString &senderId, const QString &content) {
    QLabel *lbl = new QLabel(content);
    lbl->setWordWrap(true);
    lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QWidget *container = new QWidget();
    container->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *lay = new QVBoxLayout(container);
    lay->setContentsMargins(5,5,5,5);
    lay->addWidget(lbl);

    QListWidgetItem *item = new QListWidgetItem();
    messagesListWidget->addItem(item);
    messagesListWidget->setItemWidget(item, container);

    // — Usar los mismos estilos de burbuja que en FriendsMessageWindow —
    if (senderId != userID) {
        lbl->setStyleSheet(
            "background-color: #1D4536; color: #F9F9F4; padding: 8px; font-size: 16px;"
            "border-radius: 10px; min-width: 100px; max-width: 400px;"
            );
        lay->setAlignment(lbl, Qt::AlignLeft);
    } else {
        lbl->setStyleSheet(
            "background-color: white; color: black; padding: 8px; font-size: 16px;"
            "border-radius: 10px; min-width: 100px; max-width: 400px;"
        );
        lay->setAlignment(lbl, Qt::AlignRight);
    }

    // — Guardar en el historial —
    chatHistories[chatID].append(qMakePair(senderId, content));

    // Ajustar altura y desplazarse abajo
    QTimer::singleShot(0, this, [=]() {
        lbl->adjustSize();
        int h = lbl->height() * 0.35;
        if (h < 40) h = 40;
        item->setSizeHint(QSize(400, h + 10));
        messagesListWidget->scrollToBottom();
    });
}

/**
 * @brief Lee el token de autenticación guardado en configuración.
 * @param userKey Clave del usuario para localizar el archivo .conf.
 * @return Token como QString, o cadena vacía si no existe.
 */

QString GameMessageWindow::loadAuthToken(const QString &userKey) {
    // Construimos la misma ruta que usa LoginWindow para el .conf
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                       + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
    // Forzamos formato INI para leerlo
    QSettings settings(configPath, QSettings::IniFormat);
    QString token = settings.value("auth/token").toString();
    if (token.isEmpty()) {
        qDebug() << "GameMessageWindow: Token no encontrado en" << configPath;
    }
    return token;
}
