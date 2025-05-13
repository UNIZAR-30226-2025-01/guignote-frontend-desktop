/**
 * @file myprofilewindow.h
 * @brief Declaración de la clase MyProfileWindow, interfaz del perfil de usuario.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase MyProfileWindow proporciona una ventana donde el usuario puede visualizar
 * su perfil, incluyendo nombre, ELO, estadísticas, y cambiar su foto de perfil.
 */

#ifndef USERPROFILEWINDOW_H
#define USERPROFILEWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include "icon.h"

/**
 * @class MyProfileWindow
 * @brief Ventana de perfil de usuario.
 *
 * Representa una interfaz donde el usuario puede consultar su información personal,
 * estadísticas del juego, y modificar su foto de perfil.
 */
class UserProfileWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de perfil.
     * @param userKey Clave del usuario para autenticación.
     * @param parent Widget padre (opcional).
     */
    explicit UserProfileWindow( QWidget *parent = nullptr, const QString &userKey = "", QString friendId = "" );

private:
    // --- Elementos de la UI ---
    QVBoxLayout   *mainLayout;     ///< Layout principal de la ventana.
    QLabel        *titleLabel;     ///< Título del perfil.
    QPushButton   *closeButton;    ///< Botón para cerrar la ventana.
    Icon          *fotoPerfil;     ///< Imagen de perfil del usuario.
    QLabel        *userLabel;      ///< Etiqueta que muestra el nombre y ELO.
    QLabel        *statsLabel;     ///< Estadísticas del usuario.

    QString m_userKey;             ///< Clave del usuario actual.

    // --- Métodos de configuración de UI ---
    void setupUI();
    QHBoxLayout* createHeaderLayout();
    QVBoxLayout* createProfileLayout();
    QPixmap createCircularImage(const QPixmap &src, int size);

    // --- Backend y lógica ---
    QString loadAuthToken(const QString &userKey);

    /**
     * @brief Carga el nombre, ELO y estadísticas del usuario desde el backend.
     * @param userKey Clave del usuario.
     */
    void loadNameAndStats(const QString &userKey);

    QString friendId;
};

#endif // USERPROFILEWINDOW_H
