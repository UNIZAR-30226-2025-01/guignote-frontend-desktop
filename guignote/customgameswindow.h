/**
 * @file customgameswindow.h
 * @brief Declaración de la clase CustomGamesWindow para unirse a partidas personalizadas.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase CustomGamesWindow proporciona una interfaz gráfica que permite a los usuarios
 * visualizar y unirse a partidas multijugador personalizadas, filtrando entre todas las
 * disponibles o solo las de amigos.
 */

#ifndef CUSTOMGAMESWINDOW_H
#define CUSTOMGAMESWINDOW_H

#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QDialog>
#include <QNetworkAccessManager>
#include <QWebSocket>

/**
 * @class CustomGamesWindow
 * @brief Ventana para mostrar y unirse a partidas personalizadas.
 *
 * Esta clase hereda de QDialog y permite listar partidas creadas por otros usuarios,
 * con opción de filtrado por amigos. Proporciona también la funcionalidad para
 * conectarse a una partida concreta mediante WebSocket y autenticación.
 */
class CustomGamesWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana CustomGamesWindow.
     * @param userKey Clave del usuario para autenticación.
     * @param usr Nombre del usuario actual.
     * @param fondo ID del fondo visual usado.
     * @param parent Widget padre. Opcional.
     */
    CustomGamesWindow(const QString &userKey, QString usr, int fondo, QWidget *parent);

private:
    // Elementos de interfaz
    QVBoxLayout *mainLayout; ///< Diseño vertical principal de la ventana.
    QLabel *titleLabel; ///< Etiqueta con el título de la ventana.
    QPushButton *closeButton; ///< Botón para cerrar la ventana.
    QCheckBox *soloAmigosCheck; ///< CheckBox para filtrar solo partidas de amigos.
    QListWidget *gamesListWidget; ///< Lista de partidas disponibles.

    QString userKey; ///< Clave de usuario para autenticación.
    QString token; ///< Token JWT autenticado.
    QNetworkAccessManager *networkManager; ///< Gestor de peticiones HTTP.
    QWebSocket *webSocket = nullptr; ///< WebSocket para conexión a partidas.

    bool soloAmigos = false; ///< Indica si se filtran solo partidas de amigos.
    int fondo; ///< ID del fondo o tema visual.
    QDialog *searchingDialog = nullptr; ///< Diálogo mientras se conecta a una partida.
    QLabel *countLabel = nullptr; ///< Etiqueta para mostrar número de jugadores en cola.
    int capacidad; ///< Capacidad máxima de la partida a unirse.
    int jugadoresCola; ///< Jugadores actualmente en la cola.
    QString usr; ///< Nombre del usuario actual.
    int id; ///< ID de la partida seleccionada.

    /**
     * @brief Carga el token de autenticación desde la clave de usuario.
     * @param userKey Clave del usuario.
     * @return Token JWT válido.
     */
    QString loadAuthToken(const QString &userKey);

    /**
     * @brief Configura la interfaz gráfica de la ventana.
     */
    void setupUI();

    /**
     * @brief Obtiene todas las partidas disponibles desde el servidor.
     */
    void fetchAllGames();

    /**
     * @brief Obtiene solo las partidas disponibles de amigos.
     */
    void fetchFriendGames();

    /**
     * @brief Intenta unirse a una partida específica.
     * @param idPart Identificador de la partida.
     * @param cap Capacidad máxima de la partida.
     */
    void joinGame(QString idPart, int cap);
};

#endif // CUSTOMGAMESWINDOW_H
