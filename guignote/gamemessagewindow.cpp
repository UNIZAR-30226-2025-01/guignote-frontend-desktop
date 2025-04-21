// GameMessageWindow.cpp

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


QMap<QString, QList<QPair<QString,QString>>> GameMessageWindow::chatHistories;


GameMessageWindow::GameMessageWindow(const QString &userKey, QWidget *parent, const QString &gameID, const QString &userID)
    : QWidget(parent), chatID(gameID), userID(userID)
{
    // Ventana sin borde y estilo
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    setupUI(userKey);
    setupWebSocketConnection(userKey);

    //  — Cargar historial previo si existiera —
    networkManager = new QNetworkAccessManager(this);
    loadChatHistoryFromServer(userKey);
}

void GameMessageWindow::loadChatHistoryFromServer(const QString &userKey) {
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        qDebug() << "GameMessageWindow: No se puede cargar historial, token vacío.";
        return;
    }

    QString urlString = QString("http://188.165.76.134:8000/chat_partida/obtener/?chat_id=%1").arg(chatID);
    QNetworkRequest request((QUrl(urlString)));
    request.setRawHeader("Auth", token.toUtf8());

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error()) {
            qDebug() << "Error al obtener historial:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (!doc.isObject()) {
            qDebug() << "Respuesta de historial no es JSON válido.";
            reply->deleteLater();
            return;
        }

        QJsonArray mensajes = doc.object()["mensajes"].toArray();
        for (int i = mensajes.size() - 1; i >= 0; --i) {
            QJsonObject msg = mensajes[i].toObject();
            QString senderId = QString::number(msg["emisor"].toInt());
            QString content = msg["contenido"].toString();
            appendMessage(senderId, content);
        }


        reply->deleteLater();
    });
}


GameMessageWindow::~GameMessageWindow() {
    qDebug() << "GameMessageWindow: Destructor, cerrando WebSocket.";
    webSocket->close();
    delete webSocket;
}

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

void GameMessageWindow::onConnected() {
    qDebug() << "GameMessageWindow: WebSocket conectado.";
}

void GameMessageWindow::onDisconnected() {
    qDebug() << "GameMessageWindow: WebSocket desconectado.";
}

void GameMessageWindow::onTextMessageReceived(const QString &message) {
    qDebug() << "GameMessageWindow: Mensaje recibido:" << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "GameMessageWindow: JSON inválido.";
        return;
    }
    QJsonObject obj = doc.object();

    if (obj.contains("error")) {
        qDebug() << "Error del WebSocket:" << obj["error"].toString();
        return;
    }

    QJsonObject emisorObj = obj["emisor"].toObject();
    QString senderId = QString::number(emisorObj["id"].toInt());
    QString content = obj["contenido"].toString();
    appendMessage(senderId, content);
}


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
            "background-color: #2196F3; color: white; padding: 8px; font-size: 16px;"
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

QString GameMessageWindow::loadAuthToken(const QString &userkey) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userkey);
    QFile f(configPath);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug() << "GameMessageWindow: No se pudo abrir config.";
        return {};
    }
    QString token;
    while (!f.atEnd()) {
        QString line = f.readLine().trimmed();
        if (line.startsWith("token=")) {
            token = line.mid(QString("token=").length()).trimmed();
            break;
        }
    }
    f.close();
    if (token.isEmpty())
        qDebug() << "GameMessageWindow: Token no encontrado en config.";
    return token;
}
