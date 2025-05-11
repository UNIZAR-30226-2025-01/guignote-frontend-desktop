/**
 * @file userprofilewindow.h
 * @brief Declaración de la clase UserProfileWindow, ventana de perfil de otro usuario.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase UserProfileWindow proporciona una ventana para visualizar el perfil de otro usuario,
 * mostrando su nombre, ELO, estadísticas y foto de perfil, obtenidos desde el backend.
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
 * @class UserProfileWindow
 * @brief Ventana de perfil de un usuario (no editable).
 *
 * Permite consultar información de otro usuario, como su ELO, estadísticas generales
 * y foto de perfil. No permite edición, a diferencia del perfil personal.
 */
class UserProfileWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de perfil.
     * @param parent Widget padre, por defecto nullptr.
     */
    explicit UserProfileWindow(QWidget *parent = nullptr);

private:
    // --- Elementos de la UI ---
    QVBoxLayout *mainLayout;        ///< Layout principal de la ventana.
    QLabel *titleLabel;             ///< Título de la ventana.
    QPushButton *closeButton;       ///< Botón para cerrar la ventana.
    Icon *fotoPerfil;               ///< Imagen circular del perfil del usuario.
    QLabel *userLabel;              ///< Muestra el nombre y ELO del usuario.
    QLabel *statsLabel;             ///< Muestra estadísticas obtenidas del servidor.
    QPushButton *logOutButton;      ///< Botón de cierre de sesión (oculto o desactivado en este caso).

    // --- Métodos privados para construir la interfaz ---
    void setupUI();
    QHBoxLayout* createHeaderLayout();
    QVBoxLayout* createProfileLayout();
    QHBoxLayout* createBottomLayout();

    /**
     * @brief Genera una imagen circular a partir de una ruta.
     * @param imagePath Ruta de la imagen a cargar.
     * @param size Diámetro deseado de la imagen circular.
     * @return Pixmap con forma circular.
     */
    QPixmap createCircularImage(const QString &imagePath, int size);

    // --- Métodos de comunicación con backend ---
    /**
     * @brief Carga el token de autenticación para futuras peticiones.
     * @return Token del usuario autenticado.
     */
    QString loadAuthToken();

    /**
     * @brief Consulta el nombre, ELO y estadísticas del usuario desde el servidor.
     */
    void loadNameAndStats();
};

#endif // USERPROFILEWINDOW_H
