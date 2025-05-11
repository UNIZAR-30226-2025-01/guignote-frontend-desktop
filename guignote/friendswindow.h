/**
 * @file friendswindow.h
 * @brief Declaración de la clase friendswindow para la gestión de amigos.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase friendswindow proporciona una interfaz gráfica para visualizar amigos,
 * solicitudes de amistad y realizar búsquedas de usuarios para agregar nuevos contactos.
 */

#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QCloseEvent>
#include <QJsonObject>
#include "icon.h"

/**
 * @class friendswindow
 * @brief Ventana de diálogo para la gestión de amigos y solicitudes.
 *
 * Permite visualizar la lista de amigos, gestionar solicitudes de amistad y buscar nuevos usuarios.
 */
class friendswindow : public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructor de la ventana de amigos.
     * @param userKey Clave del usuario para autenticación.
     * @param parent Widget padre.
     */
    explicit friendswindow(const QString &userKey, QWidget *parent = nullptr);

public slots:
    /**
     * @brief Elimina a un amigo de la lista.
     * @param friendId ID del amigo a eliminar.
     */
    void removeFriend(const QString &friendId);

signals:
    /**
     * @brief Señal emitida cuando cambia el número de solicitudes pendientes.
     * @param count Nuevo número de solicitudes.
     */
    void friendRequestsCountChanged(int count);

private:
    // --- Layout principal y encabezado ---
    QVBoxLayout *mainLayout;       ///< Diseño vertical principal.
    QLabel *titleLabel;            ///< Etiqueta de título de la ventana.
    QPushButton *closeButton;      ///< Botón para cerrar la ventana.
    QTabWidget *tabWidget;         ///< Pestañas principales.

    // --- Pestaña "Amigos" ---
    QListWidget *friendsListWidget;    ///< Lista de amigos actuales.

    // --- Pestaña "Solicitudes" ---
    QListWidget *requestsListWidget;   ///< Lista de solicitudes recibidas.
    QPushButton *acceptButton;         ///< Botón para aceptar solicitud.
    QPushButton *rejectButton;         ///< Botón para rechazar solicitud.

    // --- Pestaña "Buscar" ---
    QLineEdit *searchLineEdit;         ///< Campo de texto para buscar usuarios.
    QPushButton *searchButton;         ///< Botón para iniciar la búsqueda.
    QListWidget *searchResultsListWidget; ///< Resultados de búsqueda.

    QString currentSearchQuery;        ///< Consulta actual de búsqueda.

    QNetworkAccessManager *networkManager; ///< Gestor de peticiones HTTP.

    QString userKey;                   ///< Clave del usuario actual.

    // --- Métodos para configurar la interfaz ---
    /**
     * @brief Configura la interfaz de usuario.
     */
    void setupUI();

    /**
     * @brief Crea la pestaña de amigos.
     * @return Widget de la pestaña.
     */
    QWidget* createFriendsTab();

    /**
     * @brief Crea la pestaña de solicitudes.
     * @return Widget de la pestaña.
     */
    QWidget* createRequestsTab();

    /**
     * @brief Crea la pestaña de búsqueda de usuarios.
     * @return Widget de la pestaña.
     */
    QWidget* createSearchTab();

    // --- Métodos para crear widgets personalizados ---
    /**
     * @brief Crea un widget de resultado de búsqueda a partir de datos JSON.
     * @param usuario Objeto JSON con información del usuario.
     * @return Widget representando al usuario.
     */
    QWidget* createSearchResultWidget(const QJsonObject &usuario);

    /**
     * @brief Crea un widget para una solicitud de amistad.
     * @param solicitud Objeto JSON con datos de la solicitud.
     * @return Widget representando la solicitud.
     */
    QWidget* createRequestWidget(const QJsonObject &solicitud);

    /**
     * @brief Crea un widget para un amigo.
     * @param amigo Objeto JSON con los datos del amigo.
     * @return Widget representando al amigo.
     */
    QWidget* createFriendWidget(const QJsonObject &amigo);

    // --- Métodos de conexión con el backend ---
    /**
     * @brief Carga el token de autenticación desde almacenamiento local.
     * @return Token del usuario.
     */
    QString loadAuthToken();

    /** @brief Realiza una petición para obtener la lista de amigos. */
    void fetchFriends();

    /** @brief Realiza una petición para obtener solicitudes pendientes. */
    void fetchRequests();

    /** @brief Realiza una búsqueda de usuarios en el backend. */
    void searchUsers();

    /** @brief Envía una solicitud de amistad. */
    void sendFriendRequest();

    /** @brief Acepta una solicitud de amistad. */
    void acceptRequest();

    /** @brief Rechaza una solicitud de amistad. */
    void rejectRequest();

    /**
     * @brief Descarga y establece el avatar de un usuario.
     * @param imageUrl URL de la imagen.
     * @param avatarIcon Widget de tipo Icon donde se mostrará.
     */
    void downloadAndSetAvatar(const QString &imageUrl, Icon *avatarIcon);

    /**
     * @brief Crea una imagen circular a partir de un QPixmap.
     * @param src Imagen original.
     * @param size Tamaño del círculo.
     * @return Imagen circular.
     */
    QPixmap createCircularImage(const QPixmap &src, int size);
};

#endif // FRIENDSWINDOW_H
