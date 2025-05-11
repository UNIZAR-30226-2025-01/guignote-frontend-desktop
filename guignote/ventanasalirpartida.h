/**
 * @file ventanasalirpartida.h
 * @brief Declaración de la clase VentanaInfo para mostrar mensajes informativos al usuario.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase VentanaInfo representa una ventana simple que muestra un mensaje informativo
 * al usuario, con la posibilidad de ejecutar una acción de salida mediante un callback.
 */

#ifndef VENTANASALIRPARTIDA_H
#define VENTANASALIRPARTIDA_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @class VentanaInfo
 * @brief Ventana informativa con mensaje y acción de salida.
 *
 * Esta clase hereda de QWidget y muestra un mensaje al usuario junto con una opción
 * de salida definida mediante una función callback.
 */
class VentanaInfo : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor de VentanaInfo.
     * @param msg Mensaje que se mostrará en la ventana.
     * @param onSalir Función que se ejecutará al cerrar la ventana o confirmar la salida.
     * @param parent Widget padre. Opcional.
     */
    explicit VentanaInfo(const QString& msg, std::function<void()> onSalir, QWidget* parent = nullptr);

private:
    std::function<void()> onSalirCallback; ///< Callback que se ejecuta al salir.
};

#endif // VENTANASALIRPARTIDA_H
