/**
 * @file rejoinwindow.h
 * @brief Declaración de la clase RejoinWindow, interfaz para reconectarse a partidas previas.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase RejoinWindow permite al usuario visualizar las partidas disponibles para reconexión
 * y volver a unirse a una de ellas si fue desconectado previamente.
 */

#ifndef REJOINWINDOW_H
#define REJOINWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QWebSocket>

/**
 * @class RejoinWindow
 * @brief Ventana que permite al usuario reconectarse a una partida en curso.
 *
 * Muestra una lista de partidas recuperadas desde el backend y permite al usuario seleccionar una
 * para reincorporarse. Utiliza WebSocket para comunicarse con el servidor.
 */
class RejoinWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de reconexión.
     * @param jsonArray Lista de partidas a las que el usuario puede reincorporarse.
     * @param fondo Skin visual actual.
     * @param userKey Clave del usuario.
     * @param usr Nombre del usuario.
     * @param parent Widget padre (opcional).
     */
    explicit RejoinWindow(QJsonArray jsonArray, int fondo, QString &userKey, QString usr, QWidget *parent = nullptr);

private:
    /**
     * @brief Configura la interfaz gráfica.
     */
    void setupUI();

    /**
     * @brief Crea el layout superior de la ventana con el título y el botón de cierre.
     * @return Layout horizontal con encabezado.
     */
    QHBoxLayout* createHeaderLayout();

    /**
     * @brief Llena la lista con las salas disponibles.
     */
    void populateSalas();

    /**
     * @brief Maneja los mensajes recibidos del servidor.
     * @param userKey Clave del usuario.
     * @param mensaje Contenido del mensaje.
     */
    void manejarMensaje(const QString &userKey, const QString &mensaje);

    /**
     * @brief Carga el token de autenticación del usuario.
     * @param userKey Clave del usuario.
     * @return Token válido.
     */
    QString loadAuthToken(const QString &userKey);

    /**
     * @brief Realiza la solicitud de reconexión a una sala específica.
     * @param idPart ID de la partida a la que se desea reincorporar.
     */
    void rejoin(QString idPart);

    // --- Atributos privados ---
    QJsonArray salas;              ///< Lista de partidas disponibles para reconexión.
    QPushButton *closeButton;      ///< Botón para cerrar la ventana.
    QLabel *titleLabel;            ///< Título de la ventana.
    QVBoxLayout *mainLayout;       ///< Layout vertical principal.
    int fondo;                     ///< Estilo visual actual.
    QString usr;                   ///< Nombre de usuario.
    QString userKey;               ///< Clave del usuario.
    int id;                        ///< ID interno del jugador (si aplica).
    QString token;                 ///< Token de autenticación.
    QWebSocket *webSocket = nullptr; ///< Conexión WebSocket para comunicación en tiempo real.
};

#endif // REJOINWINDOW_H
