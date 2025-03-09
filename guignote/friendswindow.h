#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTabWidget>

/**
 * @brief Ventana de amigos.
 *
 * La clase friendswindow representa la interfaz de usuario para la ventana de amigos.
 * Permite visualizar y gestionar la lista de amigos, así como buscar entre ellos.
 */
class friendswindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de amigos.
     * @param parent Widget padre, por defecto es nullptr.
     */
    explicit friendswindow(QWidget *parent = nullptr);

private:
    QPushButton *closeButton;   ///< Botón para cerrar la ventana.
    QLabel *titleLabel;         ///< Etiqueta que muestra el título de la ventana.
    QLineEdit *searchBar;       ///< Barra de búsqueda para filtrar amigos.
    QVBoxLayout *mainLayout;    ///< Layout principal que organiza los widgets verticalmente.
    QTabWidget *tabWidget;      ///< Widget para manejar pestañas de amigos y solicitudes.

    /**
     * @brief Configura la interfaz de usuario.
     *
     * Este método se encarga de crear y posicionar todos los elementos gráficos
     * de la ventana de amigos, definiendo el layout, los botones, la etiqueta del título
     * y la barra de búsqueda.
     */
    void setupUI();

    /**
     * @brief Crea la pestaña de amigos.
     * @return Un puntero a la pestaña de amigos.
     */
    QWidget* createFriendsTab();

    /**
     * @brief Crea la pestaña de solicitudes de amistad.
     * @return Un puntero a la pestaña de solicitudes de amistad.
     */
    QWidget* createRequestsTab();
};

#endif // FRIENDSWINDOW_H
