/**
 * @file imagebutton.h
 * @brief Declaración de la clase ImageButton, un botón gráfico interactivo.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * La clase ImageButton permite representar un botón con imagen y texto superpuesto,
 * que responde a eventos del ratón mediante efectos visuales y señales.
 */


#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QLabel>
#include <QPixmap>
#include <QSize>
#include <QMouseEvent>
#include <QEnterEvent>  // Agregar para Qt 6

/**
 * @brief Botón de imagen con texto y efectos interactivos.
 *
 * La clase ImageButton hereda de QLabel y permite mostrar una imagen junto a un texto.
 * Además, implementa efectos interactivos (como oscurecer la imagen) al pasar el ratón
 * o hacer clic, emitiendo una señal para notificar la interacción.
 */
class ImageButton : public QLabel {
    Q_OBJECT

public:
    /**
     * @brief Constructor del botón de imagen.
     * @param imagePath Ruta de la imagen que se mostrará.
     * @param text Texto que se mostrará sobre o junto a la imagen.
     * @param parent Widget padre, por defecto es nullptr.
     *
     * Inicializa el botón de imagen cargando la imagen normal y configurando el texto.
     */
    explicit ImageButton(const QString &imagePath, const QString &text, QWidget *parent = nullptr);

    /**
     * @brief Actualiza el tamaño del botón en función de una nueva altura.
     * @param h Nueva altura para el botón.
     * @return El tamaño actualizado del botón.
     *
     * Este método ajusta el tamaño del botón y de la imagen interna en función
     * de la altura proporcionada, manteniendo la relación de aspecto.
     */
    QSize updatesize(int h);

signals:
    /**
     * @brief Señal emitida cuando se hace clic sobre el botón.
     */
    void clicked();

protected:
    /**
     * @brief Evento que se invoca al entrar el cursor en el área del botón.
     * @param event Evento de entrada del ratón (QEnterEvent).
     *
     * Se utiliza para aplicar efectos visuales (por ejemplo, mostrar la imagen oscurecida).
     */
    void enterEvent(QEnterEvent *event) override;

    /**
     * @brief Evento que se invoca al salir el cursor del área del botón.
     * @param event Evento de salida del ratón.
     *
     * Restaura la imagen a su estado normal cuando el cursor abandona el área.
     */
    void leaveEvent(QEvent *event) override;

    /**
     * @brief Evento que se invoca al presionar el botón del ratón.
     * @param event Evento de presión del ratón (QMouseEvent).
     *
     * Detecta clics y emite la señal clicked() cuando se presiona el botón del ratón.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief Genera una versión oscurecida de la imagen fuente.
     * @param source Imagen original a la que se aplicará el efecto de oscurecimiento.
     * @return QPixmap Imagen resultante con el efecto aplicado.
     *
     * Este método crea una nueva imagen a partir de la imagen original aplicando
     * un filtro o transformación para oscurecerla, utilizado para efectos interactivos.
     */
    QPixmap generateDarkenedPixmap(const QPixmap &source);

private:
    QPixmap normalPixmap;   ///< Imagen original del botón.
    QLabel *textLabel;      ///< Etiqueta para mostrar el texto asociado al botón.
    QPixmap darkenedPixmap; ///< Imagen oscurecida utilizada para efectos interactivos.
};

#endif // IMAGEBUTTON_H
