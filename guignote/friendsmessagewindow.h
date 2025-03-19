#ifndef FRIENDSMESSAGEWINDOW_H
#define FRIENDSMESSAGEWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QListWidget>

class FriendsMessageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsMessageWindow(QWidget *parent = nullptr, QString ID = "", QString Usuario = "");

private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QLineEdit *messageInput;
    QString friendID;
    QString usr;
    QNetworkAccessManager *networkManager;
    QListWidget *messagesListWidget;
    QTimer *messageTimer;

    void setupUI();
    QString loadAuthToken();
    void adjustMessageSize(QListWidgetItem *item, QLabel *messageLabel);

    void sendMessage();
    void loadMessages();

};

#endif // FRIENDSMESSAGEWINDOW_H
