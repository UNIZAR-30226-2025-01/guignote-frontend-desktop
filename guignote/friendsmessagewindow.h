/**
 * @file friendsmessagewindow.h
 * @brief Declaración de la clase FriendsMessageWindow para gestionar la mensajería entre amigos.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase FriendsMessageWindow proporciona una interfaz gráfica para enviar y recibir mensajes
 * entre usuarios, utilizando WebSockets y peticiones de red.
 */

#ifndef FRIENDSMESSAGEWINDOW_H
#define FRIENDSMESSAGEWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QBoxLayout>

/**
 * @class FriendsMessageWindow
 * @brief Ventana de chat entre el usuario actual y un amigo.
 *
 * Esta clase permite enviar, recibir y mostrar mensajes entre usuarios a través de una conexión WebSocket.
 */
class FriendsMessageWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de mensajes.
     * @param userKey Clave de usuario local (para autenticación).
     * @param friendId Identificador del amigo.
     * @param friendName Nombre del amigo a mostrar.
     * @param parent Widget padre.
     */
    explicit FriendsMessageWindow(const QString &userKey,
                                  const QString &friendId,
                                  const QString &friendName,
                                  QWidget *parent = nullptr);

    /** @brief Destructor de la clase. */
    ~FriendsMessageWindow();

signals:
    /**
     * @brief Señal emitida al recibir un nuevo mensaje.
     * @param fromFriendId Identificador del amigo que envió el mensaje.
     */
    void newMessageReceived(const QString& fromFriendId);

private slots:
    /** @brief Slot llamado al conectar el WebSocket. */
    void onConnected();

    /** @brief Slot llamado al desconectar el WebSocket. */
    void onDisconnected();

    /**
     * @brief Slot llamado al recibir un mensaje en bruto desde el WebSocket.
     * @param rawMessage Cadena JSON con el contenido del mensaje.
     */
    void onTextMessageReceived(const QString &rawMessage);

private:
    /**
     * @brief Configura la interfaz gráfica de usuario.
     * @param userKey Clave del usuario para mostrar la ventana adecuada.
     */
    void setupUI(const QString &userKey);

    /**
     * @brief Establece la conexión WebSocket con el servidor.
     * @param userKey Clave del usuario para autenticación.
     */
    void setupWebSocketConnection(const QString &userKey);

    /**
     * @brief Carga mensajes anteriores entre el usuario y el amigo.
     * @param userKey Clave del usuario.
     */
    void loadMessages(const QString &userKey);

    /**
     * @brief Envía el mensaje introducido por el usuario.
     * @param userKey Clave del usuario.
     */
    void sendMessage(const QString &userKey);

    /**
     * @brief Añade un mensaje a la lista mostrada en la UI.
     * @param senderId ID del remitente.
     * @param content Contenido textual del mensaje.
     */
    void appendMessage(const QString &senderId, const QString &content);

    /**
     * @brief Recupera el token de autenticación desde almacenamiento local.
     * @param userKey Clave del usuario.
     * @return Token de autenticación.
     */
    QString loadAuthToken(const QString &userKey);

    /**
     * @brief Recupera el identificador del usuario desde almacenamiento local.
     * @param userKey Clave del usuario.
     * @return ID del usuario.
     */
    QString loadOwnId(const QString &userKey);

    // --- Componentes de red ---
    QWebSocket               *webSocket;        ///< Cliente WebSocket para mensajes en tiempo real.
    QNetworkAccessManager    *networkManager;   ///< Gestor de peticiones HTTP.

    // --- Interfaz de usuario ---
    QVBoxLayout  *mainLayout;           ///< Diseño vertical principal.
    QLabel       *titleLabel;           ///< Etiqueta con el nombre del amigo.
    QPushButton  *closeButton;          ///< Botón para cerrar la ventana.
    QListWidget  *messagesListWidget;   ///< Lista de mensajes mostrados.
    QLineEdit    *messageInput;         ///< Campo de texto para escribir mensajes.
    QPushButton  *sendButton;           ///< Botón para enviar mensajes.
    QSet<QString> m_shownKeys;          ///< Conjunto de claves de mensajes ya mostrados (evita duplicados).
    QDateTime     wsIgnoreUntil;        ///< Tiempo hasta el cual se ignoran mensajes WebSocket (por reconexión).

    // --- Datos del chat ---
    QString friendID;     ///< Identificador del amigo.
    QString usr;          ///< Nombre de usuario (posible alias).
    QString ownID;        ///< ID del usuario local.
    QString m_userKey;    ///< Clave del usuario para autenticación.
};

#endif // FRIENDSMESSAGEWINDOW_H
