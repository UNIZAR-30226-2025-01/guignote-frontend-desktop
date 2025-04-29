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
#include <qgraphicseffect.h>
#include "icon.h"
#include "mano.h"
#include "deck.h"
#include "posicion.h"
#include "menuwindow.h"




class GameWindow : public QWidget // Ensure GameWindow inherits from QWidget
{
    Q_OBJECT
public:
    GameWindow(const QString &userKey, int type, int fondo, QJsonObject msg, int id, QWebSocket *ws, QString usr, MenuWindow *menuRef);
    void addCartaPorId(Carta *c);
    Carta* getCartaPorId(QString id);

private:
    std::function<void()> pendingRoundResult;

    bool myTurn = false; // true si es mi turno, false en caso contrario
    int winPileCountUser = 0;
    int winPileCountOpponent = 0;
    static constexpr int winPileOffset = 15;  // desplazamiento entre cartas apiladas
    static constexpr int winPileMargin = 20;  // margen respecto a la esquina
    QHash<Posicion*, int> pilaCount;
    QMap<int,int> playerPosMap;
    bool arrastre = false;
    QTimer *hideOptionsTimer = nullptr;
    bool isMouseOverOptions = false;
    QString gameID;
    int bg; // Number that indicates which skin of the background is being used [0,1,2...]
    int gameType; // Number that indicates whether the game is 1v1, 2v2, friendly, or normal.
    int cardSize;
    int player_id;
    QString usr;
    QString token;
    QWebSocket *ws;

    // 0 -> 1v1 Ranked
    // 1 -> 1v1 Friendly
    // 2 -> 2v2 (Ranked or Friendly)

    bool eventFilter(QObject *watched, QEvent *event) override;
    void setBackground(); // Function to set the background based on the bg value
    void setupUI(const QString &userKey);
    void setupGameElements(QJsonObject msg);
    void resizeEvent(QResizeEvent *event) override;
    void repositionOrnaments();
    void repositionOptions();
    void repositionHands();
    QString loadAuthToken(const QString &userKey);
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
    QString chatID;
    QPropertyAnimation *showOptionsAnimation = nullptr;
    QPropertyAnimation *hideOptionsAnimation = nullptr;
    bool isOptionsVisible = false;
    int optionsBarHeight = 80;  // altura fija de la barra
    const int indicatorHeight  = 6;   // alto del indicador
    QLabel   *optionsIndicator = nullptr;
    QGraphicsOpacityEffect *overlayEffect = nullptr;
    QPropertyAnimation *fadeIn = nullptr;
    QPropertyAnimation *fadeOut = nullptr;
    QWidget *overlay = nullptr;
    QLabel *turnoLabel = nullptr;
    QGraphicsOpacityEffect *overlayOpacity = nullptr;
    QPropertyAnimation *fadeAnimation = nullptr;

    void mostrarTurno(const QString &texto, bool miTurno);
    void ocultarTurno();
    void getSettings();
    MenuWindow *menuWindowRef = nullptr;

};

#endif // GAMEWINDOW_H
