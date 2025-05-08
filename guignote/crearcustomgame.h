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

class CrearCustomGame : public QDialog
{
    Q_OBJECT
public:
    CrearCustomGame(QString &userKey, QDialog *parent);
private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QCheckBox *soloAmigos;
    QCheckBox *t15s;
    QCheckBox *t30s;
    QCheckBox *t60s;
    QCheckBox *revueltas;
    QCheckBox *arrastre;

    bool soloAmigosB = false;
    int tiempo = 15;
    bool revueltasB = true;
    bool arrastreB = true;

    void setupUI();
    void crearPartida();
    QString loadAuthToken(const QString &userKey);

    QString token;
    QNetworkAccessManager *networkManager;
};

#endif // CREARCUSTOMGAME_H
