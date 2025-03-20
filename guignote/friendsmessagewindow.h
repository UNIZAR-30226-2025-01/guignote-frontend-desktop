#ifndef FRIENDSMESSAGEWINDOW_H
#define FRIENDSMESSAGEWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QListWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebSocket>

class FriendsMessageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsMessageWindow(QWidget *parent = nullptr, QString ID = "", QString Usuario = "");
    ~FriendsMessageWindow();

private:
    QWebSocket *webSocket;
    void onTextMessageReceived(const QString &message);

    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QLineEdit *messageInput;
    QString friendID;
    QString usr;
    QNetworkAccessManager *networkManager;
    QListWidget *messagesListWidget;

    void setupUI();
    void setupWebSocketConnection();
    QString loadAuthToken();
    void adjustMessageSize(QListWidgetItem *item, QLabel *messageLabel);

    void sendMessage();
    void loadMessages();
    void onConnected();
    void onDisconnected();
};

#endif // FRIENDSMESSAGEWINDOW_H
