#ifndef FRIENDSMESSAGEWINDOW_H
#define FRIENDSMESSAGEWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

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

    void setupUI();
    QString loadAuthToken();

    void sendMessage();
    void loadMessages();
};

#endif // FRIENDSMESSAGEWINDOW_H
