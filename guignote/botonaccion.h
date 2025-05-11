/**
 * @file botonaccion.h
 * @brief Declaración de la clase BotonAccion para botones con acción personalizada.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase BotonAccion extiende QPushButton y permite asociar directamente
 * una función callback a la acción de clic.
 */

#ifndef BOTONACCION_H
#define BOTONACCION_H

#include <QPushButton>
#include <functional>

/**
 * @class BotonAccion
 * @brief Botón de interfaz gráfica que ejecuta una función al hacer clic.
 *
 * Esta clase personaliza el comportamiento de un QPushButton, permitiendo
 * pasar una función (`std::function<void()>`) que se ejecutará automáticamente
 * cuando se suelte el botón.
 */
class BotonAccion : public QPushButton {
    Q_OBJECT

public:
    /**
     * @brief Constructor de BotonAccion.
     * @param texto Texto que se mostrará en el botón.
     * @param callback Función que se ejecutará al soltar el botón. Opcional.
     * @param parent Widget padre. Opcional.
     */
    explicit BotonAccion(const QString& texto,
                         std::function<void()> callback = nullptr, QWidget* parent = nullptr);

protected:
    /**
     * @brief Evento que se dispara al soltar el botón del ratón.
     * @param event Evento de tipo QMouseEvent.
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    std::function<void()> callback; ///< Función que se ejecuta al soltar el botón.
};

#endif // BOTONACCION_H
