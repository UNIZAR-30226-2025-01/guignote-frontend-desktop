#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QMap>

class ChatManager : public QObject {
    Q_OBJECT
public:
    static ChatManager& instance();
    // Arranca el manager con tu JWT; debe llamarse antes de suscribir a nadie
    void start(const QString &jwtToken);
    // Suscribe (y abre un WS) para ese friendId; idempotente
    void subscribeTo(const QString &friendId);

signals:
    // Se emite cada vez que llega un mensaje por WS
    void messageReceived(const QString &fromId, const QString &content);

private slots:
    void onSocketConnected();
    void onSocketTextMessage(const QString &raw);

private:
    explicit ChatManager(QObject *parent = nullptr);
    QString jwt;
    QMap<QString, QWebSocket*> sockets;
};

#endif // CHATMANAGER_H
