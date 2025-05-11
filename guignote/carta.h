/**
 * @file carta.h
 * @brief Declaración de la clase Carta para el proyecto de Qt.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Define la clase Carta, que representa una carta gráfica con capacidad
 * de interacción (drag & drop), giro y asociación a una mano de cartas.
 */

#ifndef CARTA_H
#define CARTA_H

#include <QLabel>
#include <QPixmap>
#include <QPoint>

class Mano;
class GameWindow;

/**
 * @class Carta
 * @brief Clase que representa una carta interactiva en la interfaz gráfica.
 *
 * Hereda de QLabel y permite mostrar cartas con imagen de anverso/reverso,
 * arrastrarlas, asociarlas a una mano y controlar su visibilidad.
 */
class Carta : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief Constructor de la carta.
     * @param gw Puntero a la ventana principal del juego.
     * @param parent Widget padre.
     * @param num Número o figura de la carta.
     * @param suit Palo de la carta.
     * @param h Altura en píxeles.
     * @param skin Estilo gráfico de la carta.
     * @param faceUp Determina si se muestra el anverso (true) o reverso (false).
     */
    explicit Carta(GameWindow *gw = nullptr,
                   QWidget *parent = nullptr,
                   const QString &num = "0",
                   const QString &suit = "",
                   int h = 100,
                   int skin = 0,
                   bool faceUp = true);

    /** @brief Muestra el anverso de la carta. */
    void reveal();

    /** @brief Muestra el reverso de la carta. */
    void hideFace();

    /**
     * @brief Asocia la carta a una mano.
     * @param mano Puntero a la mano.
     * @param id Identificador único dentro de la mano.
     */
    void añadirAMano(Mano* mano, int id);

    /** @brief Elimina la carta de la mano a la que pertenece. */
    void eliminarDeMano();

    /**
     * @brief Establece si la carta está bloqueada para interacción.
     * @param lock True para bloquear, false para desbloquear.
     */
    void setLock(bool lock);

    /**
     * @brief Obtiene la imagen actual de la carta.
     * @return Pixmap actual.
     */
    QPixmap getImagen() const;

    /**
     * @brief Establece la imagen de la carta.
     * @param pixmap Imagen a establecer.
     * @param h Altura deseada para la carta.
     */
    void setImagen(const QPixmap &pixmap, int h);

    QString idGlobal;  ///< Identificador global único ("num" + "suit").
    QString num;       ///< Número o figura de la carta.
    QString suit;      ///< Palo de la carta.

    /**
     * @brief Devuelve el pixmap original sin modificaciones.
     * @return Pixmap original.
     */
    QPixmap getOriginalPixmap() const;

protected:
    /** @brief Gestiona el evento de pulsación del ratón. */
    void mousePressEvent(QMouseEvent *event) override;

    /** @brief Gestiona el evento de movimiento del ratón. */
    void mouseMoveEvent(QMouseEvent *event) override;

    /** @brief Gestiona el evento de liberación del ratón. */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    /**
     * @brief Selecciona el pixmap de anverso según el skin.
     * @param skin Estilo gráfico.
     * @return Pixmap correspondiente al skin.
     */
    QPixmap selectPixmap(int skin) const;

    QPixmap frontPixmap;     ///< Imagen del anverso.
    QPixmap backPixmap;      ///< Imagen del reverso.
    QPixmap imagen;          ///< Imagen actualmente mostrada.
    QPixmap pixmapOrig;      ///< Imagen original sin modificaciones.

    bool isFaceUp;           ///< Indica si la carta muestra el anverso.
    bool locked;             ///< Indica si la carta está bloqueada.
    bool arrastrando;        ///< Indica si se está arrastrando.
    QPoint offsetArrastre;   ///< Desplazamiento respecto al cursor.
    int ID;                  ///< ID interno (por ejemplo, en la mano).
    Mano* mano;              ///< Puntero a la mano a la que pertenece.
};

#endif // CARTA_H
