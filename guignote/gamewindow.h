/**
 * @file gamewindow.h
 * @brief Declaración de la clase GameWindow, ventana principal de la partida.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * GameWindow representa la interfaz principal de una partida multijugador,
 * gestionando la lógica de juego, las animaciones, los sonidos y la comunicación
 * en tiempo real con el servidor.
 */

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
#include <QGraphicsEffect>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "icon.h"
#include "mano.h"
#include "deck.h"
#include "posicion.h"
#include "menuwindow.h"

/**
 * @class GameWindow
 * @brief Ventana principal de la partida.
 *
 * Gestiona el flujo de la partida, la interfaz gráfica, el drag & drop de cartas,
 * las animaciones de turno, los efectos de sonido y la comunicación WebSocket.
 */
class GameWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de juego.
     * @param userKey Clave de usuario para autenticación.
     * @param type Tipo de partida (1v1, 2v2, amistoso, etc.).
     * @param fondo Índice del estilo de fondo.
     * @param msg Estado inicial de la partida en formato JSON.
     * @param id Identificador interno del jugador.
     * @param ws Puntero al WebSocket de comunicación.
     * @param usr Nombre del usuario.
     * @param menuRef Puntero a la ventana de menú que lanzó esta partida.
     */
    GameWindow(const QString &userKey,
               int type,
               int fondo,
               QJsonObject msg,
               int id,
               QWebSocket *ws,
               QString usr,
               MenuWindow *menuRef);

    /**
     * @brief Añade una carta al mapa interno por su ID.
     * @param c Puntero a la carta a añadir.
     */
    void addCartaPorId(Carta *c);

    /**
     * @brief Obtiene una carta por su identificador.
     * @param id Identificador global de la carta.
     * @return Puntero a la carta, o nullptr si no existe.
     */
    Carta* getCartaPorId(QString id);

    /** @brief Destructor que limpia recursos. */
    ~GameWindow();

private:
    /**
     * @brief Procesa de forma segura el resultado de la ronda recibido en JSON.
     * @param data Objeto JSON con los datos del resultado de la ronda.
     */
    void procesarRoundResultSeguro(const QJsonObject& data);

    QSequentialAnimationGroup *currentRoundAnim = nullptr; ///< Animación de la ronda actual.
    std::function<void()> pendingRoundResult;            ///< Callback pendiente tras animación.
    QTimer *hideTurnoTimer;                              ///< Temporizador para ocultar indicador de turno.
    bool myTurn = false;                                 ///< Indica si es el turno del jugador.
    int winPileCountUser = 0;                            ///< Cartas ganadas por el usuario.
    int winPileCountOpponent = 0;                        ///< Cartas ganadas por el oponente.
    int deckSkin;                                        ///< Skin de las cartas.
    static constexpr int winPileOffset = 15;             ///< Offset entre cartas apiladas.
    static constexpr int winPileMargin = 20;             ///< Margen de la pila respecto a la esquina.
    QHash<Posicion*, int> pilaCount;                     ///< Conteo de cartas por posición.
    QMap<int,int> playerPosMap;                          ///< Mapa de posiciones de jugadores.
    bool arrastre = false;                               ///< Indica si se está arrastrando carta.
    QTimer *hideOptionsTimer = nullptr;                  ///< Temporizador para ocultar opciones.
    bool isMouseOverOptions = false;                     ///< Indica si el ratón está sobre el menú de opciones.
    QJsonObject pendingRoundResultData;                  ///< Datos de resultado de ronda pendientes.

    QString gameID;                                      ///< Identificador de la partida.
    int bg;                                              ///< Índice del fondo seleccionado.
    int gameType;                                        ///< Tipo de partida.
    int cardSize;                                        ///< Tamaño en píxeles de las cartas.
    int player_id;                                       ///< ID del jugador local.
    QString usr;                                         ///< Nombre del usuario.
    QString token;                                       ///< Token de autenticación.
    QWebSocket *ws;                                      ///< WebSocket de comunicación.

    QMediaPlayer *backgroundPlayer = nullptr;            ///< Reproductor de música de fondo.
    QAudioOutput *audioOutput = nullptr;                 ///< Control de volumen de la música.
    QMediaPlayer *sfxPlayer;                             ///< Reproductor de efectos de sonido.
    QAudioOutput *sfxOutput;                             ///< Control de volumen de efectos.

    QVector<int> winPileCounts = QVector<int>(4, 0);     ///< Conteo de pilas ganadoras por posición.

    /**
     * @brief Filtra eventos globales (usado para opciones flotantes).
     * @param watched Objeto observado.
     * @param event Evento detectado.
     * @return true si se procesa, false en caso contrario.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /** @brief Ajusta el fondo de la ventana según el valor de bg. */
    void setBackground();

    /**
     * @brief Configura la interfaz de usuario.
     * @param userKey Clave de usuario para personalizar la UI.
     */
    void setupUI(const QString &userKey);

    /**
     * @brief Inicializa los elementos de juego a partir del estado JSON.
     * @param msg JSON con el estado de la partida.
     */
    void setupGameElements(QJsonObject msg);

    /** @brief Maneja el evento de redimensionamiento. */
    void resizeEvent(QResizeEvent *event) override;

    /** @brief Reposiciona los adornos en las esquinas de la ventana. */
    void repositionOrnaments();

    /** @brief Reposiciona el panel de opciones. */
    void repositionOptions();

    /** @brief Reposiciona las manos de los jugadores. */
    void repositionHands();

    /**
     * @brief Carga el token de autenticación.
     * @param userKey Clave del usuario.
     * @return Token de autenticación.
     */
    QString loadAuthToken(const QString &userKey);

    /** @brief Obtiene el ID del jugador del servidor. */
    void getID();

    /** @brief Obtiene el nombre de usuario del servidor. */
    void getUsr();

    /**
     * @brief Procesa mensajes recibidos en el chat de partida.
     * @param mensaje Texto del mensaje recibido.
     */
    void recibirMensajes(const QString &mensaje);

    /** @brief Muestra o carga la leyenda si procede. */
    void colocarLeyenda();

    // Labels de adornos en esquinas
    QLabel *cornerTopLeft;     ///< Adorno esquina superior izquierda.
    QLabel *cornerTopRight;    ///< Adorno esquina superior derecha.
    QLabel *cornerBottomLeft;  ///< Adorno esquina inferior izquierda.
    QLabel *cornerBottomRight; ///< Adorno esquina inferior derecha.

    // Iconos de acciones
    Icon *settings; ///< Icono de ajustes.
    Icon *chat;     ///< Icono de chat.
    Icon *quit;     ///< Icono de abandonar partida.

    // Barra de opciones flotantes
    QFrame *optionsBar;        ///< Contenedor de opciones.
    QSize ornamentSize;        ///< Tamaño de los adornos de esquina.

    // Elementos de juego
    QVector<Mano*> manos;           ///< Manos de los jugadores.
    QVector<Posicion*> posiciones;  ///< Posiciones en la mesa.
    static QMap<QString, Carta*> cartasPorId; ///< Mapa global de cartas por ID.
    Deck *deck;                     ///< Mazo de cartas.

    /**
     * @brief Inicializa el estado de juego a partir de JSON.
     * @param s0 JSON con el estado inicial.
     */
    void setupGameState(QJsonObject s0);

    QString chatID;                      ///< Identificador del chat de partida.
    QPropertyAnimation *showOptionsAnimation = nullptr; ///< Animación para mostrar opciones.
    QPropertyAnimation *hideOptionsAnimation = nullptr; ///< Animación para ocultar opciones.
    bool isOptionsVisible = false;       ///< Indica si el menú de opciones está visible.
    int optionsBarHeight = 80;           ///< Altura de la barra de opciones.
    const int indicatorHeight = 6;       ///< Alto del indicador de turno.
    QLabel *optionsIndicator = nullptr;  ///< Indicador de turno.
    QGraphicsOpacityEffect *overlayEffect = nullptr; ///< Efecto de opacidad para overlay.
    QPropertyAnimation *fadeIn = nullptr; ///< Animación de aparición del overlay.
    QPropertyAnimation *fadeOut = nullptr; ///< Animación de desaparición del overlay.
    QWidget *overlay = nullptr;           ///< Widget overlay.
    QLabel *turnoLabel = nullptr;         ///< Etiqueta que muestra "Tu turno".
    QGraphicsOpacityEffect *overlayOpacity = nullptr; ///< Efecto de opacidad de turno.
    QPropertyAnimation *fadeAnimation = nullptr; ///< Animación final de turno.

    /**
     * @brief Muestra la indicación de turno con texto.
     * @param texto Texto a mostrar.
     * @param miTurno True si es el turno del jugador.
     */
    void mostrarTurno(const QString &texto, bool miTurno);

    /** @brief Oculta la indicación de turno. */
    void ocultarTurno();

    /** @brief Recupera ajustes desde la ventana de menú. */
    void getSettings();

    MenuWindow *menuWindowRef = nullptr; ///< Puntero a la ventana de menú.

    // Simulación de montones de cartas ganadoras
    QMap<int, QVector<Carta*>> pileBacks; ///< Vectores de backs por posición.
    const int pileBackOffset = 8;        ///< Offset entre backs del montón.

    bool roundResultInProgress = false; ///< Indica animación de resultado en curso.
    QJsonObject pendingTurnUpdateData;  ///< Datos JSON pendientes de actualización de turno.

    /**
     * @brief Procesa actualizaciones de turno.
     * @param data JSON con los datos de actualización.
     */
    void processTurnUpdate(const QJsonObject &data);

    QJsonObject pendingDrawData; ///< Datos JSON de robos pendientes.

    /**
     * @brief Anima el robo de cartas.
     * @param drawData JSON con los datos del robo.
     * @param userID ID del jugador que robó.
     */
    void animateDraw(const QJsonObject &drawData, int userID);

    bool hasPendingDraw = false;       ///< Indica si hay robo pendiente.
    int pendingDrawUserId = -1;        ///< ID del usuario con robo pendiente.
    QVector<int> pendingDrawUserIds;   ///< Cola de usuarios con robos pendientes.

    QLabel *legendLabel = nullptr; ///< Etiqueta de leyenda.
    bool legendPinned = false;     ///< True si la leyenda está fija.
};

#endif // GAMEWINDOW_H
