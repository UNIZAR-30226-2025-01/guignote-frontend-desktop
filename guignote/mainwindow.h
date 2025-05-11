/**
 * @file mainwindow.h
 * @brief Declaración de la clase MainWindow, la ventana principal de la aplicación.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase MainWindow gestiona la interfaz principal de la aplicación,
 * incluyendo la navegación hacia las ventanas de inicio de sesión y registro,
 * así como la gestión de decoraciones visuales en la ventana principal.
 */

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
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /** @brief Destructor de la clase MainWindow. */
    ~MainWindow();

private slots:
    /** @brief Abre la ventana de inicio de sesión. */
    void openLoginWindow();

    /** @brief Abre la ventana de registro de usuario. */
    void openRegisterWindow();

    /** @brief Maneja la solicitud de abrir la ventana de inicio desde la de registro. */
    void handleOpenLoginRequested();

    /** @brief Maneja la solicitud de abrir la ventana de registro desde la de inicio. */
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
     * @param event Evento de tipo `QResizeEvent`.
     */
    void resizeEvent(QResizeEvent *event) override;

    /** @brief Reposiciona los adornos en las esquinas tras redimensionar. */
    void repositionOrnaments();
};

#endif // MAINWINDOW_H
