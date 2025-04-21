// GameMessageWindow.h

#pragma once

#include <QWidget>
#include <QWebSocket>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <qboxlayout.h>
#include <qlabel.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class GameMessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit GameMessageWindow(const QString &userKey, QWidget *parent, const QString &chatID, const QString &userID);
    ~GameMessageWindow();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void sendMessage(const QString &userKey);

private:

    QNetworkAccessManager *networkManager;
    void loadChatHistoryFromServer(const QString &userKey);
    void setupUI(const QString userKey);
    void setupWebSocketConnection(const QString &userKey);
    void appendMessage(const QString &senderId, const QString &content);
    QString loadAuthToken(const QString &userkey);
    // Historial estático de chats por partida
    static QMap<QString, QList<QPair<QString,QString>>> chatHistories;
    QString chatID;
    QString userID;
    QWebSocket *webSocket;
    // Identificador de este chat, para indexar chatHistories
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QListWidget *messagesListWidget;
    QLineEdit *messageInput;
    QPushButton *sendButton;
};
