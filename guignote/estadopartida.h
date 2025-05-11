/**
 * @file estadopartida.h
 * @brief Declaración de la clase EstadoPartida para gestionar el estado de una partida multijugador.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase EstadoPartida gestiona toda la lógica visual y de comunicación relacionada con
 * el desarrollo de una partida, incluyendo procesamiento de eventos, interacción con WebSocket,
 * gestión de turnos, puntuaciones y control del flujo del juego.
 */

#ifndef ESTADOPARTIDA_H
#define ESTADOPARTIDA_H

#include "carta.h"
#include "mano.h"
#include "botonaccion.h"
#include <QWidget>
#include <QJsonObject>
#include <QTimer>
#include <QtWebSockets/QWebSocket>
#include <QQueue>

/**
 * @struct Jugador
 * @brief Estructura que representa un jugador dentro de la partida.
 */
struct Jugador {
    QString nombre; ///< Nombre del jugador.
    int id; ///< Identificador único del jugador.
    int equipo; ///< Número de equipo al que pertenece.
    Mano* mano; ///< Mano actual del jugador.
    int numCartas; ///< Número de cartas en la mano.
};

/**
 * @class EstadoPartida
 * @brief Widget que representa el estado completo de una partida de cartas multijugador.
 *
 * Esta clase hereda de QWidget y gestiona toda la lógica visual y funcional de una partida.
 * Se encarga de recibir eventos por WebSocket, actualizar el estado de juego,
 * mostrar puntuaciones, manejar pausas y mostrar overlays de espera o mensajes.
 */
class EstadoPartida : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor de EstadoPartida.
     * @param miNombre Nombre del jugador local.
     * @param wsUrl URL del servidor WebSocket.
     * @param bg Fondo visual (opcional).
     * @param style Estilo de cartas (opcional).
     * @param onSalir Función callback que se ejecuta al salir de la partida.
     * @param parent Widget padre (opcional).
     */
    EstadoPartida(QString miNombre, const QString& wsUrl, int bg = 0, int style = 0,
                  std::function<void()> onSalir = nullptr, QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~EstadoPartida();

    /**
     * @brief Inicializa el estado de la partida.
     */
    void init();

    /**
     * @brief Establece el ID del jugador local.
     * @param id Identificador del jugador.
     */
    void setMiId(int id);

    /**
     * @brief Verifica si la partida ha comenzado.
     * @return true si la partida está iniciada, false en caso contrario.
     */
    bool getPartidaIniciada() const;

    /**
     * @brief Establece el estado de inicio de la partida.
     * @param partidaIniciada Nuevo valor del estado.
     */
    void setPartidaIniciada(bool partidaIniciada);

    /**
     * @brief Redibuja el estado actual de la partida.
     */
    void dibujarEstado();

    std::function<void()> onSalir; ///< Función callback ejecutada al salir de la partida.

    // Procesamiento de eventos desde el servidor
    void procesarStartGame(QJsonObject data);
    void procesarCardPlayed(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarCardDrawn(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarTurnUpdate(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarRoundResult(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarPhaseUpdate(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarEndGame(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarPause(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarResume(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarAllPause(QJsonObject data, std::function<void()> callback = nullptr);
    void procesarError(QJsonObject data, std::function<void()> callback);
    void procesarCambioSiete(QJsonObject data, std::function<void()> callback);
    void procesarCanto(QJsonObject data, std::function<void()> callback);

    void manejarEventoPrePartida(const QString& tipo, const QJsonObject& data);
    void mostrarOverlayEspera(int jugadoresCola, int jugadoresMax);
    void actualizarOverlayEspera(int jugadoresCola, int jugadoresMax);
    void ocultarOverlayEspera();

    void mostrarMensaje(const QString& msg, std::function<void()> callback, int duracion = 1250);

    void actualizarPuntuacion(int equipo, int nuevoValor, std::function<void()> callback);

    void procesarMensajeWebSocket(const QString& mensaje);

    // Cola de eventos
    void recibirEvento(const QJsonObject& evento);
    void procesarSiguienteEvento();

    QString miNombre = ""; ///< Nombre del jugador local.

public slots:
    /**
     * @brief Slot llamado al hacer doble clic sobre una carta.
     * @param carta Puntero a la carta seleccionada.
     */
    void onCartaDobleClick(Carta* carta);

    /**
     * @brief Slot para realizar la acción de "cantar".
     */
    void onCantar();

    /**
     * @brief Slot para cambiar un siete.
     */
    void onCambiarSiete();

    /**
     * @brief Slot para pausar la partida.
     */
    void onPausa();

    /**
     * @brief Slot para anular la pausa.
     */
    void onAnularPausa();

private:
    // Métodos auxiliares
    void limpiar();
    void actualizarEstado(const QJsonObject& data);
    void iniciarBotonesYEtiquetas();
    void enviarMsg(QJsonObject& msg);

    // Estado del juego
    Carta* cartaTriunfo = nullptr;
    QVector<Jugador*> jugadores;
    QMap<int, Jugador*> mapJugadores;
    int puntosEquipo1 = 0;
    int puntosEquipo2 = 0;
    int mazoRestante = 0;
    int chatId, miId;
    Carta* mazo = nullptr;
    bool enPausa = false;
    int jugadoresPausa = 0;
    bool partidaIniciada = false;

    // WebSocket y eventos
    QQueue<QJsonObject> colaEventos;
    bool enEjecucion = false;
    QWebSocket* websocket = nullptr;

    // UI: overlays, botones, puntuaciones
    QWidget* overlay = nullptr;
    QLabel* overlayMsg = nullptr;

    QLabel *puntosEquipo1Label = nullptr, *puntosEquipo1Title = nullptr;
    QLabel *puntosEquipo2Label = nullptr, *puntosEquipo2Title = nullptr;

    BotonAccion* botonCantar = nullptr;
    BotonAccion* botonCambiarSiete = nullptr;
    BotonAccion* botonPausa = nullptr;
    QLabel* pausadosLabel = nullptr;

    QWidget* overlayEspera = nullptr;
    QLabel* labelEspera = nullptr;
    int jugadoresCola = 0, jugadoresMax = 0;

    QString wsUrl; ///< URL del servidor WebSocket.
};

#endif // ESTADOPARTIDA_H
