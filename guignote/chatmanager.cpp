#include "chatmanager.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ChatManager& ChatManager::instance() {
    static ChatManager inst;
    return inst;
}

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
{}

void ChatManager::start(const QString &jwtToken) {
    jwt = jwtToken;
}

void ChatManager::subscribeTo(const QString &friendId) {
    if (friendId.isEmpty() || sockets.contains(friendId))
        return;

    // Construir WS URL
    QWebSocket *sock = new QWebSocket();
    QString url = QString("ws://188.165.76.134:8000/ws/chat/%1/?token=%2")
                      .arg(friendId, jwt);
    sock->open(QUrl(url));

    connect(sock, &QWebSocket::connected, this, &ChatManager::onSocketConnected);
    connect(sock, &QWebSocket::textMessageReceived,
            this, &ChatManager::onSocketTextMessage);

    sockets.insert(friendId, sock);
}

void ChatManager::onSocketConnected() {
    qDebug() << "ChatManager: WebSocket conectado.";
}

void ChatManager::onSocketTextMessage(const QString &raw) {
    // Parsear JSON y emitir
    QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();
    if (obj.contains("error")) return;

    QString fromId  = QString::number(obj["emisor"].toInt());
    QString content = obj["contenido"].toString();
    emit messageReceived(fromId, content);
}
