#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;  // Declaración adelantada de QLabel para evitar incluir todo el archivo

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Clase que representa la ventana principal de la aplicación.
 *
 * Esta clase extiende QMainWindow y se encarga de gestionar la interfaz principal,
 * incluyendo la apertura de ventanas secundarias como la de inicio de sesión (`LoginWindow`)
 * y la de registro (`RegisterWindow`). También maneja decoraciones y eventos de redimensionamiento.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la clase MainWindow.
     * @param parent Widget padre de la ventana (opcional, por defecto `nullptr`).
     *
     * Inicializa la interfaz gráfica y configura los elementos visuales principales.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor de la clase MainWindow.
     *
     * Se encarga de liberar los recursos utilizados por la interfaz de usuario (`ui`).
     */
    ~MainWindow();

private slots:
    /**
     * @brief Abre la ventana de inicio de sesión.
     *
     * Se activa cuando el usuario pulsa el botón "Iniciar sesión".
     */
    void openLoginWindow();

    /**
     * @brief Abre la ventana de registro de usuario.
     *
     * Se activa cuando el usuario pulsa el botón "Crear cuenta".
     */
    void openRegisterWindow();

    /**
     * @brief Maneja la solicitud de abrir la ventana de inicio de sesión desde la de registro.
     *
     * Se conecta con la señal emitida por `RegisterWindow` cuando el usuario pulsa
     * "¿Ya tienes cuenta?".
     */
    void handleOpenLoginRequested();

    /**
     * @brief Maneja la solicitud de abrir la ventana de registro desde la de inicio de sesión.
     *
     * Se conecta con la señal emitida por `LoginWindow` cuando el usuario pulsa
     * "¿No tienes cuenta?".
     */
    void handleOpenRegisterRequested();

private:
    Ui::MainWindow *ui; ///< Puntero a la interfaz gráfica generada por Qt Designer.

    // --- Elementos decorativos de la ventana ---
    QLabel *cornerTopLeft;     ///< Imagen decorativa en la esquina superior izquierda.
    QLabel *cornerTopRight;    ///< Imagen decorativa en la esquina superior derecha.
    QLabel *cornerBottomLeft;  ///< Imagen decorativa en la esquina inferior izquierda.
    QLabel *cornerBottomRight; ///< Imagen decorativa en la esquina inferior derecha.

    QSize ornamentSize; ///< Tamaño de los adornos en las esquinas de la ventana.

    /**
     * @brief Evento de redimensionamiento de la ventana.
     * @param event Evento de tipo `QResizeEvent` que contiene los nuevos valores de tamaño.
     *
     * Se ejecuta automáticamente cuando la ventana cambia de tamaño y ajusta la posición de los adornos.
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief Reposiciona los adornos en las esquinas de la ventana.
     *
     * Se llama desde `resizeEvent()` para mantener la alineación correcta
     * de los elementos decorativos cuando la ventana cambia de tamaño.
     */
    void repositionOrnaments();
};

#endif // MAINWINDOW_H
