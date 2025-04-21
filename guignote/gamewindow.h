#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QLabel>      // For QLabel class
#include <QPixmap>     // For QPixmap class
#include <QSize>       // For QSize class
#include <QTransform>  // For QTransform class
#include <QNetworkAccessManager>
#include <QCloseEvent>
#include <QJsonObject>
#include <QtWebSockets>
#include <QUuid>
#include "icon.h"
#include "mano.h"
#include "deck.h"
#include "posicion.h"


class GameWindow : public QWidget // Ensure GameWindow inherits from QWidget
{
    Q_OBJECT
public:
    GameWindow(int type, int fondo, QJsonObject msg, int id, QWebSocket *ws);
    void addCartaPorId(Carta *c);
    Carta* getCartaPorId(QString id);

private:
    QString gameID;
    int bg; // Number that indicates which skin of the background is being used [0,1,2...]
    int gameType; // Number that indicates whether the game is 1v1, 2v2, friendly, or normal.
    int cardSize;
    int player_id;
    QString token;
    QWebSocket *ws;

    // 0 -> 1v1 Ranked
    // 1 -> 1v1 Friendly
    // 2 -> 2v2 (Ranked or Friendly)

    void setBackground(); // Function to set the background based on the bg value
    void setupUI();
    void setupGameElements(QJsonObject msg);
    void resizeEvent(QResizeEvent *event);
    void repositionOrnaments();
    void repositionOptions();
    void repositionHands();
    QString loadAuthToken();
    void getID();
    void getUsr();
    void recibirMensajes(const QString &mensaje);

    // Labels to display the corner ornaments
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;

    // Icons
    Icon *settings;
    Icon *chat;
    Icon *quit;

    //Barra
    QFrame *optionsBar;

    QSize ornamentSize; // Size of the corner ornaments

    // Game Elements
    QVector<Mano*> manos;
    QVector<Posicion*> posiciones;
    static QMap<QString, Carta*> cartasPorId;
    Deck *deck;

    // Conexión Backend
    void setupGameState(QJsonObject s0);
};

#endif // GAMEWINDOW_H
