#ifndef USERPROFILEWINDOW_H
#define USERPROFILEWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include "icon.h"

/**
 * @brief Ventana de perfil de usuario.
 *
 * La clase UserProfileWindow representa la interfaz de usuario para el perfil de un usuario.
 * Muestra la foto de perfil, el nombre, el ELO y estadísticas obtenidas desde el backend.
 */
class UserProfileWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de perfil.
     * @param parent Widget padre, por defecto es nullptr.
     */
    explicit UserProfileWindow(QWidget *parent = nullptr);

private:
    // Elementos de la UI
    QVBoxLayout *mainLayout;    ///< Layout principal.
    QLabel *titleLabel;         ///< Etiqueta con el título de la ventana.
    QPushButton *closeButton;   ///< Botón para cerrar la ventana.
    Icon *fotoPerfil;           ///< Widget que muestra la foto de perfil (imagen circular).
    QLabel *userLabel;          ///< Etiqueta para mostrar el nombre y el ELO (actualizado desde el backend).
    QLabel *statsLabel;         ///< Etiqueta para mostrar las estadísticas.
    QPushButton *logOutButton;  ///< Botón para cerrar sesión (Log Out).

    // Métodos para configurar la interfaz de usuario.
    void setupUI();
    QHBoxLayout* createHeaderLayout();
    QVBoxLayout* createProfileLayout();
    QHBoxLayout* createBottomLayout();
    QPixmap createCircularImage(const QString &imagePath, int size);

    // Métodos para la conexión con el backend.
    QString loadAuthToken();
    void loadNameAndStats();
};

#endif // USERPROFILEWINDOW_H
