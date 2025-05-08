#ifndef CREARCUSTOMGAME_H
#define CREARCUSTOMGAME_H

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

class CrearCustomGame : public QDialog
{
    Q_OBJECT
public:
    CrearCustomGame(QString &userKey, QString usr, int fondo, QDialog *parent);
private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QCheckBox *soloAmigos;
    QCheckBox *ind;
    QCheckBox *par;
    QCheckBox *t15s;
    QCheckBox *t30s;
    QCheckBox *t60s;
    QCheckBox *revueltas;
    QCheckBox *arrastre;

    QWebSocket *webSocket;
    QLabel *mensajeCola = nullptr;
    QDialog *searchingDialog = nullptr;
    int jugadoresCola = 0;
    int jugadoresMax = 0;
    QString userKey;
    QLabel* countLabel = nullptr;
    QString usr;
    int id;
    int fondo;

    bool soloAmigosB = false;
    int tiempo = 15;
    bool individual = true;
    bool revueltasB = true;
    bool arrastreB = true;

    void setupUI();
    void crearPartida();
    QString loadAuthToken(const QString &userKey);
    void manejarMensaje(const QString &userKey, const QString &mensaje);

    QString token;
    QNetworkAccessManager *networkManager;
};

#endif // CREARCUSTOMGAME_H
