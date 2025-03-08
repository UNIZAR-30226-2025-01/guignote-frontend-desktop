#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
<<<<<<< HEAD
#include <QLineEdit>
#include <QTabWidget>
=======
#include <QLineEdit>  // Asegúrate de incluir esta librería
>>>>>>> 244881bf257fb2b25504870b34102437d14eaf16

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
<<<<<<< HEAD
    QPushButton *closeButton;
    QLabel *titleLabel;
    QLineEdit *searchBar;
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;

    void setupUI();  // Configura la UI
    QWidget* createFriendsTab();   // Crea la pestaña de amigos
    QWidget* createRequestsTab();  // Crea la pestaña de solicitudes de amistad
=======
    QPushButton *closeButton;   ///< Botón para cerrar la ventana.
    QLabel *titleLabel;         ///< Etiqueta que muestra el título de la ventana.
    QLineEdit *searchBar;       ///< Barra de búsqueda para filtrar amigos.
    QVBoxLayout *mainLayout;    ///< Layout principal que organiza los widgets verticalmente.

    /**
     * @brief Configura la interfaz de usuario.
     *
     * Este método se encarga de crear y posicionar todos los elementos gráficos
     * de la ventana de amigos, definiendo el layout, los botones, la etiqueta del título
     * y la barra de búsqueda.
     */
    void setupUI();
>>>>>>> 244881bf257fb2b25504870b34102437d14eaf16
};

#endif // FRIENDSWINDOW_H
