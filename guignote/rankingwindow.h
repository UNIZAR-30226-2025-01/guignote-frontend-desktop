/**
 * @file rankingwindow.h
 * @brief Declaración de la clase RankingWindow, ventana de rankings del juego.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase RankingWindow permite visualizar los rankings individuales y por parejas,
 * tanto globales como de amigos, recuperando los datos desde el servidor mediante peticiones HTTP.
 */

#ifndef RANKINGWINDOW_H
#define RANKINGWINDOW_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QJsonArray>

/**
 * @class RankingWindow
 * @brief Ventana que muestra los rankings individuales y por parejas.
 *
 * Permite al usuario consultar clasificaciones globales y filtrarlas por "solo amigos".
 */
class RankingWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de RankingWindow.
     * @param userKey Clave de autenticación del usuario.
     * @param parent Widget padre (opcional).
     */
    explicit RankingWindow(const QString &userKey, QWidget *parent = nullptr);

private:
    QString currentUserName;         ///< Nombre del usuario actual (opcional para resaltar en ranking).

    // --- Elementos UI ---
    QVBoxLayout *mainLayout;         ///< Diseño vertical principal.
    QLabel *titleLabel;              ///< Título de la ventana.
    QPushButton *closeButton;        ///< Botón para cerrar la ventana.
    QPushButton *individualButton;   ///< Botón para ver ranking individual.
    QPushButton *parejasButton;      ///< Botón para ver ranking por parejas.
    QCheckBox *soloAmigosCheck;      ///< Checkbox para filtrar solo amigos.
    QListWidget *rankingListWidget;  ///< Lista que muestra los jugadores y sus datos.

    // --- Red ---
    QNetworkAccessManager *networkManager; ///< Gestor de peticiones HTTP.
    QString authToken;                     ///< Token de autenticación del usuario.

    // --- Estado y lógica ---
    QString amigos = "";           ///< Cadena que representa filtro de amigos (si se aplica).
    int lastPressed = 1;           ///< 1 para individual, 2 para parejas.

    // --- Métodos privados ---
    /** @brief Configura la interfaz gráfica. */
    void setupUI();

    /**
     * @brief Carga el token de autenticación del usuario.
     * @param userKey Clave del usuario.
     * @return Token válido para uso en peticiones.
     */
    QString loadAuthToken(const QString &userKey);

    /** @brief Solicita el ranking individual desde el backend. */
    void fetchIndividualRanking();

    /** @brief Solicita el ranking por parejas desde el backend. */
    void fetchTeamRanking();

    /** @brief Maneja la respuesta HTTP del ranking individual. */
    void handleIndividualRankingResponse();

    /** @brief Maneja la respuesta HTTP del ranking por parejas. */
    void handleTeamRankingResponse();

    /**
     * @brief Actualiza la lista de jugadores mostrados en pantalla.
     * @param playersArray Array de jugadores en formato JSON.
     * @param type Tipo de ranking (1 = individual, 2 = parejas).
     */
    void updateRankingList(const QJsonArray &playersArray, int type);
};

#endif // RANKINGWINDOW_H
