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
    explicit FriendsMessageWindow( const QString &userKeyQWidget, QWidget *parent = nullptr, QString ID = "", QString Usuario = "");
    ~FriendsMessageWindow();

private:
    QWebSocket *webSocket;
    void onTextMessageReceived(const QString &message, const QString &userKey);
    void addMessageToList(const QString &contenido, int emisorId);
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QLineEdit *messageInput;
    QString friendID;
    QString usr;
    QNetworkAccessManager *networkManager;
    QListWidget *messagesListWidget;

    void setupUI(const QString &userKey);
    void setupWebSocketConnection(const QString &userKey);
    QString loadAuthToken(const QString &userKey);
    void adjustMessageSize(QListWidgetItem *item, QLabel *messageLabel);

    void sendMessage(const QString &userKey);
    void loadMessages(const QString &userKey);
    void onConnected();
    void onDisconnected();
};

#endif // FRIENDSMESSAGEWINDOW_H
