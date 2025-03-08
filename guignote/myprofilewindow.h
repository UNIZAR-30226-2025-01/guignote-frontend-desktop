#ifndef MYPROFILEWINDOW_H
#define MYPROFILEWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QListWidget>


/**
 * @brief Ventana de amigos.
 *
 * La clase friendswindow representa la interfaz de usuario para la ventana de amigos.
 * Permite visualizar y gestionar la lista de amigos, así como buscar entre ellos.
 */
class MyProfileWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la ventana de amigos.
     * @param parent Widget padre, por defecto es nullptr.
     */
    explicit MyProfileWindow(QWidget *parent = nullptr);

private:
    QPushButton *closeButton;   ///< Botón para cerrar la ventana.
    QLabel *titleLabel;         ///< Etiqueta que muestra el título de la ventana.
    QVBoxLayout *mainLayout;    ///< Layout principal que organiza los widgets verticalmente.

    /**
     * @brief Configura la interfaz de usuario.
     *
     * Este método se encarga de crear y posicionar todos los elementos gráficos
     * de la ventana de amigos, definiendo el layout, los botones, la etiqueta del título
     * y la barra de búsqueda.
     */
    void setupUI();
};
#endif // MYPROFILEWINDOW_H
