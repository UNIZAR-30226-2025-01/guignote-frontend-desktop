/**
 * @file menuwindow.h
 * @brief Declaración de la clase MenuWindow, ventana principal del menú del juego.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * MenuWindow representa la interfaz principal tras iniciar sesión, permitiendo
 * al usuario acceder a modos de juego, ajustes, amigos, inventario y rankings,
 * así como gestionar la conexión con el servidor para emparejamientos y reconexiones.
 */

#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QSize>
#include <QLabel>
#include <QFrame>
#include <QMediaPlayer>
#include <QtMultimedia>
#include <QAudioOutput>
#include <QResizeEvent>
#include <QEvent>
#include <QPushButton>
#include <QWebSocket>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QTimer>

class ImageButton;
class Icon;

namespace Ui {
class MenuWindow;
}

/**
 * @class MenuWindow
 * @brief Ventana principal del menú de la aplicación.
 *
 * Gestiona la interfaz que permite al usuario navegar entre modos de juego,
 * acceder a la configuración, ver la lista de amigos, rankings e inventario.
 * También reproduce música de fondo y mantiene la conexión WebSocket para
 * emparejamientos y reconexiones de partidas.
 */
class MenuWindow : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor de MenuWindow.
     * @param userKey Clave de usuario para autenticación.
     * @param parent Widget padre (opcional).
     */
    explicit MenuWindow(const QString &userKey, QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~MenuWindow();

    QMediaPlayer *backgroundPlayer = nullptr; ///< Reproductor de música de fondo.
    QAudioOutput *audioOutput = nullptr;      ///< Control de volumen de la música.

public slots:
    /**
     * @brief Ajusta el volumen de la música de fondo.
     * @param volumePercentage Volumen entre 0 y 100.
     */
    void setVolume(int volumePercentage);

protected:
    /**
     * @brief Evento de redimensionamiento de la ventana.
     * @param event Evento de tipo QResizeEvent.
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief Evento al cerrar la ventana.
     * @param event Evento de tipo QCloseEvent.
     */
    void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Evento al mostrar la ventana.
     * @param event Evento de tipo QShowEvent.
     */
    void showEvent(QShowEvent *event) override;

private:
    Ui::MenuWindow *ui; ///< Interfaz generada por Qt Designer.

    // Botones de modos de juego
    ImageButton *boton1v1; ///< Botón para partida 1v1.
    ImageButton *boton2v2; ///< Botón para partida 2v2.

    // Barras de la interfaz
    QFrame *bottomBar; ///< Barra inferior.
    QFrame *topBar;    ///< Barra superior.

    // Iconos de opciones
    Icon *settings;   ///< Icono de ajustes.
    Icon *friends;    ///< Icono de amigos.
    Icon *exit;       ///< Icono para salir de la sesión.
    Icon *inventory;  ///< Icono de inventario.
    Icon *rankings;   ///< Icono de rankings.

    QPushButton *invisibleButton; ///< Botón oculto para interacciones internas.
    QLabel *usrLabel;             ///< Etiqueta que muestra el nombre de usuario.

    // Adornos decorativos
    QLabel *cornerTopLeft;     ///< Adorno esquina superior izquierda.
    QLabel *cornerTopRight;    ///< Adorno esquina superior derecha.
    QLabel *cornerBottomLeft;  ///< Adorno esquina inferior izquierda.
    QLabel *cornerBottomRight; ///< Adorno esquina inferior derecha.
    QSize ornamentSize;        ///< Tamaño de los adornos.

    // Reposicionamiento de elementos UI
    void repositionOrnaments();     ///< Reposiciona los adornos de esquina.
    void repositionImageButtons();  ///< Reposiciona los botones de imagen.
    void repositionBars();          ///< Reposiciona las barras.
    void repositionIcons();         ///< Reposiciona los iconos.
    void getSettings();             ///< Carga y aplica ajustes visuales.

    // Conexión con backend
    /**
     * @brief Carga el token de autenticación.
     * @param userKey Clave de usuario.
     * @return Token de autenticación.
     */
    QString loadAuthToken(const QString &userKey);

    QString token;    ///< Token de autenticación.

    QWebSocket *webSocket; ///< Cliente WebSocket para comunicación en tiempo real.

    /**
     * @brief Solicita y gestiona el emparejamiento de una partida.
     * @param userKey Clave de usuario.
     * @param token Token de autenticación.
     * @param capacidad Número de jugadores de la partida.
     */
    void jugarPartida(const QString &userKey, const QString &token, int capacidad = 2);

    QLabel *mensajeCola = nullptr;      ///< Mensaje informativo mientras busca partida.
    QDialog *searchingDialog = nullptr; ///< Diálogo de espera de emparejamiento.
    int jugadoresCola = 0;              ///< Jugadores actualmente en cola.
    int jugadoresMax = 0;               ///< Jugadores máximos permitidos.

    /**
     * @brief Procesa mensajes recibidos del servidor.
     * @param userKey Clave de usuario.
     * @param mensaje Contenido del mensaje.
     */
    void manejarMensaje(const QString &userKey, const QString &mensaje);

    QString usr;      ///< Nombre del usuario.
    QString userKey;  ///< Clave de usuario.
    int id;           ///< Identificador interno de usuario.

    QPushButton *ReconnectButton = nullptr; ///< Botón para reintentar conexión.

    QJsonArray salas;        ///< Lista de salas disponibles para reconexión.
    QVBoxLayout *mainLayout; ///< Layout principal del menú.

    /**
     * @brief Comprueba si existen partidas pendientes para reconectarse.
     */
    void checkRejoin();

    QLabel *countLabel = nullptr; ///< Etiqueta para mostrar número de partidas/cola.

    void ensureLoggedIn(); ///< Verifica que el usuario esté autenticado.
    QString loadToken();   ///< Carga el token desde almacenamiento local.
    void saveToken(const QString &tok); ///< Guarda el token localmente.

    Icon *friendsIcon = nullptr;   ///< Icono alternativo de amigos.
    Icon *messagesIcon = nullptr;  ///< Icono de mensajes.
    int unreadMessages = 0;        ///< Contador de mensajes sin leer.

    int fondo = 1;         ///< Skin de fondo seleccionado.
    QTimer *rejoinTimer;   ///< Temporizador para intentos de reconexión.
};

#endif // MENUWINDOW_H
