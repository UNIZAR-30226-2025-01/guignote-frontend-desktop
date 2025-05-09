#ifndef FRIENDSMESSAGEWINDOW_H
#define FRIENDSMESSAGEWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <qboxlayout.h>

class FriendsMessageWindow : public QWidget
{
    Q_OBJECT

public:
    // Ahora recibe: userKey, friendId, friendName, [parent]
    explicit FriendsMessageWindow(const QString &userKey,
                                  const QString &friendId,
                                  const QString &friendName,
                                  QWidget *parent = nullptr);
    ~FriendsMessageWindow();
signals:
    // Emite cada vez que llegue un mensaje nuevo
    void newMessageReceived(const QString& fromFriendId);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &rawMessage);

private:
    // Inicialización
    void setupUI(const QString &userKey);
    void setupWebSocketConnection(const QString &userKey);

    // Mensajes
    void loadMessages(const QString &userKey);
    void sendMessage(const QString &userKey);
    void appendMessage(const QString &senderId, const QString &content);

    // Autenticación
    QString loadAuthToken(const QString &userKey);
    QString loadOwnId(const QString &userKey);

    // Componentes de red
    QWebSocket               *webSocket;
    QNetworkAccessManager    *networkManager;

    // UI
    QVBoxLayout  *mainLayout;
    QLabel       *titleLabel;
    QPushButton  *closeButton;
    QListWidget  *messagesListWidget;
    QLineEdit    *messageInput;
    QPushButton  *sendButton;

    // Datos de chat
    QString friendID;
    QString usr;
    QString ownID;
    QString m_userKey;     // guarda aquí el userKey
};

#endif // FRIENDSMESSAGEWINDOW_H
