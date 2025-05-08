#ifndef CUSTOMGAMESWINDOW_H
#define CUSTOMGAMESWINDOW_H

#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QDialog>
#include <QNetworkAccessManager>

class CustomGamesWindow : public QDialog
{
    Q_OBJECT
public:
    CustomGamesWindow(const QString &userKey, QWidget *parent);
private:
    // UI
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QCheckBox *soloAmigosCheck;
    QListWidget *gamesListWidget;

    QString userKey;
    QString token;
    QNetworkAccessManager *networkManager;

    QString loadAuthToken(const QString &userKey);
    void setupUI();
    bool soloAmigos = false;

    void fetchAllGames();
    void fetchFriendGames();
};

#endif // CUSTOMGAMESWINDOW_H
