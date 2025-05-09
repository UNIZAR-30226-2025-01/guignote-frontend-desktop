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
#include <QWebSocket>
#include <QString>

class CustomGamesWindow : public QDialog
{
    Q_OBJECT
public:
    CustomGamesWindow(const QString &userKey, QString usr, int fondo, QWidget *parent);
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
    QWebSocket *webSocket = nullptr;

    QString loadAuthToken(const QString &userKey);
    void setupUI();
    bool soloAmigos = false;
    int fondo;
    QDialog *searchingDialog = nullptr;
    QLabel *countLabel = nullptr;
    int capacidad;
    int jugadoresCola;

    QString usr;
    int id;

    void fetchAllGames();
    void fetchFriendGames();
    void manejarMensaje(const QString &userKey, const QString &mensaje);
    void joinGame(QString idPart);
};

#endif // CUSTOMGAMESWINDOW_H
