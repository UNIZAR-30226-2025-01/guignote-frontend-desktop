#ifndef MYPROFILEWINDOW_H
#define MYPROFILEWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include "icon.h"

/**
 * @brief Ventana de perfil de usuario.
 *
 * La clase MyProfileWindow representa la interfaz de usuario para el perfil personal,
 * mostrando la foto de perfil, el nombre, ELO y estadísticas obtenidas del backend.
 */
class MyProfileWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de perfil.
     * @param parent Widget padre, por defecto es nullptr.
     */
    explicit MyProfileWindow(const QString &userKey, QWidget *parent = nullptr);

private:
    // Elementos de la UI
    QVBoxLayout   *mainLayout;      ///< Layout principal.
    QLabel        *titleLabel;      ///< Etiqueta con el título.
    QPushButton   *closeButton;     ///< Botón para cerrar la ventana.
    Icon          *fotoPerfil;      ///< Widget que muestra la foto de perfil.
    QLabel        *userLabel;       ///< Etiqueta para mostrar el nombre y el ELO.
    QLabel        *statsLabel;      ///< Etiqueta para mostrar las estadísticas del usuario.
    QPushButton   *logOutButton;    ///< Botón para cerrar sesión (Log Out).

    // Métodos para configurar la interfaz
    void setupUI();
    QHBoxLayout* createHeaderLayout();
    QVBoxLayout* createProfileLayout();
    QHBoxLayout* createBottomLayout();
    QPixmap createCircularImage(const QString &imagePath, int size);

    // Métodos para conexión con el backend
    QString loadAuthToken(const QString &userKey);
    void loadNameAndStats(const QString &userKey); // Método que consulta el backend para obtener nombre, ELO y estadísticas.
};

#endif // MYPROFILEWINDOW_H
