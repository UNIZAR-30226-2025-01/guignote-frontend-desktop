#ifndef REJOINWINDOW_H
#define REJOINWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QWebSocket>

class RejoinWindow : public QDialog {
    Q_OBJECT

public:
    explicit RejoinWindow(QJsonArray jsonArray, int fondo, QString &userKey, QString usr, QWidget *parent = nullptr);

private:
    void setupUI();
    QHBoxLayout* createHeaderLayout();
    void populateSalas();
    void manejarMensaje(const QString &userKey, const QString &mensaje);
    QString loadAuthToken(const QString &userKey);
    void rejoin(QString idPart);

    QJsonArray salas;

    QPushButton *closeButton;
    QLabel *titleLabel;
    QVBoxLayout *mainLayout;
    int fondo;
    QString usr;
    QString userKey;
    int id;
    QString token;
    QWebSocket *webSocket = nullptr;
};

#endif // REJOINWINDOW_H
