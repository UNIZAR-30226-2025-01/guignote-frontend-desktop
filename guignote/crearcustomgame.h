/**
 * @file crearcustomgame.h
 * @brief Declaración de la clase CrearCustomGame para crear partidas personalizadas.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase CrearCustomGame proporciona una interfaz gráfica para configurar y lanzar
 * partidas multijugador personalizadas, con opciones como tipo de juego, tiempo, reglas,
 * y filtrado por amigos.
 */

#ifndef CREARCUSTOMGAME_H
#define CREARCUSTOMGAME_H

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
 * @class CrearCustomGame
 * @brief Diálogo para crear una partida multijugador personalizada.
 *
 * Esta clase permite al usuario configurar opciones específicas para una partida
 * personalizada, como el número de jugadores, el tiempo de turno, las reglas de juego
 * y si solo se permite la entrada de amigos. Se conecta a través de WebSocket y
 * utiliza autenticación mediante token.
 */
class CrearCustomGame : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor del diálogo CrearCustomGame.
     * @param userKey Clave de usuario para autenticación.
     * @param usr Nombre del usuario.
     * @param fondo ID del fondo o tema visual.
     * @param parent Diálogo padre. Opcional.
     */
    CrearCustomGame(QString &userKey, QString usr, int fondo, QDialog *parent);

private:
    QVBoxLayout *mainLayout; ///< Diseño principal del diálogo.
    QLabel *titleLabel; ///< Título del diálogo.
    QPushButton *closeButton; ///< Botón para cerrar el diálogo.

    // Opciones de configuración del juego
    QCheckBox *soloAmigos; ///< Opción para permitir solo amigos.
    QCheckBox *ind; ///< Opción de modo individual.
    QCheckBox *par; ///< Opción de modo por parejas.
    QCheckBox *t15s; ///< Tiempo de turno: 15 segundos.
    QCheckBox *t30s; ///< Tiempo de turno: 30 segundos.
    QCheckBox *t60s; ///< Tiempo de turno: 60 segundos.
    QCheckBox *revueltas; ///< Activar reglas de revueltas.
    QCheckBox *arrastre; ///< Activar reglas de arrastre.

    QWebSocket *webSocket; ///< WebSocket para la comunicación en tiempo real.
    QLabel *mensajeCola = nullptr; ///< Mensaje de estado al buscar jugadores.
    QDialog *searchingDialog = nullptr; ///< Diálogo mostrado mientras se busca partida.

    int jugadoresCola = 0; ///< Número actual de jugadores en la cola.
    int jugadoresMax = 0; ///< Número máximo de jugadores permitidos.

    QString userKey; ///< Clave de usuario para autenticación.
    QLabel* countLabel = nullptr; ///< Etiqueta que muestra el contador de jugadores.
    QString usr; ///< Nombre del usuario.
    int id; ///< Identificador interno de la partida.
    int fondo; ///< Identificador del fondo visual.

    // Valores internos de configuración
    bool soloAmigosB = false; ///< Indica si la partida es solo para amigos.
    int tiempo = 30; ///< Tiempo de turno (por defecto 30 segundos).
    bool individual = true; ///< Modo de juego individual.
    bool revueltasB = true; ///< Activar reglas de revueltas.
    bool arrastreB = true; ///< Activar reglas de arrastre.

    /**
     * @brief Configura los elementos de la interfaz gráfica.
     */
    void setupUI();

    /**
     * @brief Envía la configuración y crea la partida personalizada.
     */
    void crearPartida();

    /**
     * @brief Carga el token de autenticación usando la clave del usuario.
     * @param userKey Clave del usuario.
     * @return Token de autenticación.
     */
    QString loadAuthToken(const QString &userKey);

    QString token; ///< Token JWT de autenticación.
    QNetworkAccessManager *networkManager; ///< Gestor de red para operaciones HTTP.
};

#endif // CREARCUSTOMGAME_H
