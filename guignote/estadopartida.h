#ifndef ESTADOPARTIDA_H
#define ESTADOPARTIDA_H

#include "carta.h"
#include "mano.h"
#include <QWidget>
#include <QJsonObject>
#include <QTimer>
#include <QtWebSockets/QWebSocket>
#include <QQueue>
#include "botonaccion.h"

struct Jugador {
    QString nombre;
    int id;
    int equipo;
    Mano* mano;
    int numCartas;
};

class EstadoPartida : public QWidget {
    Q_OBJECT
public:
    EstadoPartida(QString miNombre, const QString& wsUrl, int bg = 0, int style = 0,
                  std::function<void()> onSalir = nullptr, QWidget* parent = nullptr);

    ~EstadoPartida();

    void init();

    void setMiId(int id) { this->miId = id; }

    bool getPartidaIniciada() const { return this->partidaIniciada; }

    void setPartidaIniciada(bool partidaIniciada) { this->partidaIniciada = partidaIniciada; }

    void dibujarEstado();

    std::function<void()> onSalir;

    // PROCESAR MENSAJES FRONTEND
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

    void manejarEventoPrePartida(const QString& tipo, const QJsonObject& data);
    void mostrarOverlayEspera(int jugadoresCola, int jugadoresMax);
    void actualizarOverlayEspera(int jugadoresCola, int jugadoresMax);
    void ocultarOverlayEspera();

    void mostrarMensaje(const QString& msg, std::function<void()> callback, int duracion=1250);

    void actualizarPuntuacion(int equipo, int nuevoValor, std::function<void()> callback);

    void procesarMensajeWebSocket(const QString& mensaje);

    // COLA DE EVENTOS
    void recibirEvento(const QJsonObject& evento);
    void procesarSiguienteEvento();

    QString miNombre = "";
public slots:
    void onCartaDobleClick(Carta* carta);
    void onCantar();
    void onCambiarSiete();
    void onPausa();
    void onAnularPausa();

private:
    // PROCEDIMIENTOS PRIVADOS
    void limpiar();
    void actualizarEstado(const QJsonObject& data);
    void iniciarBotonesYEtiquetas();

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

    // Cola de eventos y comunicación websocket
    void enviarMsg(QJsonObject& msg);

    QQueue<QJsonObject> colaEventos;
    bool enEjecucion = false;
    QWebSocket* websocket = nullptr;

    // Overlay con mensaje
    QWidget* overlay = nullptr;
    QLabel* overlayMsg = nullptr;

    // Labels puntuaciones
    QLabel *puntosEquipo1Label = nullptr, *puntosEquipo1Title = nullptr;
    QLabel *puntosEquipo2Label = nullptr, *puntosEquipo2Title = nullptr;

    // Botones cantar y cambiar siete
    BotonAccion* botonCantar = nullptr;
    BotonAccion* botonCambiarSiete = nullptr;
    BotonAccion* botonPausa = nullptr;
    QLabel* pausadosLabel = nullptr;

    // Overlay espera
    QWidget* overlayEspera = nullptr;
    QLabel* labelEspera = nullptr;
    int jugadoresCola = 0, jugadoresMax = 0;
    bool partidaIniciada = false;

    QString wsUrl;
};

#endif // ESTADOPARTIDA_H
