/**
 * @file inventorywindow.h
 * @brief Declaración de la clase InventoryWindow, interfaz de gestión de inventario.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase InventoryWindow proporciona una ventana con pestañas para gestionar
 * elementos cosméticos del usuario, como barajas y tapetes, aplicando animaciones
 * visuales al cambiar de sección.
 */

#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QButtonGroup>

/**
 * @class InventoryWindow
 * @brief Ventana de inventario para gestionar barajas y tapetes.
 *
 * Proporciona una interfaz gráfica dividida por pestañas con animaciones de transición,
 * permitiendo al usuario visualizar y seleccionar diferentes aspectos visuales del juego.
 */
class InventoryWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la clase InventoryWindow.
     * @param parent Widget padre de la ventana.
     * @param usr Nombre del usuario que accede al inventario.
     */
    explicit InventoryWindow(QWidget *parent = nullptr, QString usr = "");

    /** @brief Destructor. */
    ~InventoryWindow();

private:
    QListWidget *sidebar;            ///< Lista lateral para navegación por pestañas.
    QStackedWidget *stackedWidget;   ///< Contenedor apilado para los contenidos de cada pestaña.
    QPushButton *closeButton;        ///< Botón para cerrar la ventana.

    // Animaciones de transición
    QPropertyAnimation *fadeOutAnim; ///< Animación de desvanecimiento al salir de una pestaña.
    QPropertyAnimation *fadeInAnim;  ///< Animación de aparición al entrar a una pestaña.

    // Efectos visuales aplicados a los contenidos
    QGraphicsOpacityEffect *deckEffect; ///< Efecto de opacidad para la vista de barajas.
    QGraphicsOpacityEffect *matEffect;  ///< Efecto de opacidad para la vista de tapetes.

    // Grupos de botones para selección exclusiva
    QButtonGroup *deckGroup; ///< Grupo de botones para seleccionar baraja.
    QButtonGroup *matGroup;  ///< Grupo de botones para seleccionar tapete.

    /**
     * @brief Configura los elementos de la interfaz de usuario.
     */
    void setupUI();

private slots:
    /**
     * @brief Cambia de pestaña al seleccionar un nuevo índice en la barra lateral.
     * @param index Índice de la pestaña seleccionada.
     */
    void onTabChanged(int index);
};

#endif // INVENTORYWINDOW_H
