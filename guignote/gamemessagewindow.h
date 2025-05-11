/**
 * @file GameMessageWindow.h
 * @brief Declaración de la clase GameMessageWindow para el chat en partidas.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * GameMessageWindow proporciona una interfaz de chat en tiempo real durante una partida multijugador.
 * Utiliza WebSocket para comunicación en vivo y HTTP para cargar el historial previo.
 */

#pragma once

#include <QWidget>
#include <QWebSocket>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/**
 * @class GameMessageWindow
 * @brief Ventana de chat para partidas multijugador.
 *
 * Permite enviar y recibir mensajes en tiempo real mediante WebSocket,
 * así como recuperar el historial del chat desde el servidor.
 */
class GameMessageWindow : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor de GameMessageWindow.
     * @param userKey Clave del usuario para autenticación.
     * @param parent Widget padre.
     * @param chatID Identificador del chat (por partida).
     * @param userID ID del usuario actual.
     */
    explicit GameMessageWindow(const QString &userKey, QWidget *parent, const QString &chatID, const QString &userID);

    /** @brief Destructor de GameMessageWindow. */
    ~GameMessageWindow();

private slots:
    /** @brief Slot llamado cuando se establece conexión con el WebSocket. */
    void onConnected();

    /** @brief Slot llamado cuando se pierde la conexión con el WebSocket. */
    void onDisconnected();

    /**
     * @brief Slot llamado al recibir un mensaje desde el WebSocket.
     * @param message Mensaje recibido en formato texto.
     */
    void onTextMessageReceived(const QString &message);

    /**
     * @brief Envía el mensaje actual al servidor.
     * @param userKey Clave del usuario.
     */
    void sendMessage(const QString &userKey);

private:
    QNetworkAccessManager *networkManager;  ///< Gestor de peticiones de red.

    /**
     * @brief Carga el historial del chat desde el servidor.
     * @param userKey Clave del usuario.
     */
    void loadChatHistoryFromServer(const QString &userKey);

    /**
     * @brief Configura los elementos de la interfaz gráfica.
     * @param userKey Clave del usuario para personalización.
     */
    void setupUI(const QString userKey);

    /**
     * @brief Establece la conexión WebSocket para el chat.
     * @param userKey Clave del usuario para autenticación.
     */
    void setupWebSocketConnection(const QString &userKey);

    /**
     * @brief Añade un mensaje a la ventana de chat.
     * @param senderId ID del remitente del mensaje.
     * @param content Texto del mensaje.
     */
    void appendMessage(const QString &senderId, const QString &content);

    /**
     * @brief Carga el token de autenticación del usuario.
     * @param userkey Clave del usuario.
     * @return Token de autenticación.
     */
    QString loadAuthToken(const QString &userkey);

    // --- Atributos ---

    /** @brief Historiales estáticos de chats por partida. */
    static QMap<QString, QList<QPair<QString,QString>>> chatHistories;

    QString chatID;              ///< Identificador del chat (por partida).
    QString userID;              ///< ID del usuario local.
    QWebSocket *webSocket;       ///< Cliente WebSocket para mensajería en tiempo real.

    // Interfaz gráfica
    QVBoxLayout *mainLayout;         ///< Layout principal vertical.
    QLabel *titleLabel;              ///< Título de la ventana.
    QPushButton *closeButton;        ///< Botón para cerrar la ventana.
    QListWidget *messagesListWidget; ///< Lista de mensajes mostrados.
    QLineEdit *messageInput;         ///< Campo de texto para escribir mensajes.
    QPushButton *sendButton;         ///< Botón para enviar mensajes.
};
