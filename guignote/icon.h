#ifndef ICON_H
#define ICON_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include <QPixmap>
#include <qpropertyanimation.h>

/**
 * @brief Widget interactivo para mostrar un icono.
 *
 * La clase Icon hereda de QLabel y permite cargar y mostrar una imagen
 * con un tamaño especificado. Además, detecta clics del mouse y emite
 * una señal para notificar a otros componentes cuando se interactúa con ella.
 */
class Icon : public QLabel {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la clase Icon.
     * @param parent Widget padre, por defecto es nullptr.
     *
     * Inicializa el widget y configura las propiedades necesarias para mostrar el icono.
     */
    explicit Icon(QWidget *parent = nullptr);
    void setHoverEnabled(bool enabled);

    /**
     * @brief Carga y establece una imagen en el icono.
     * @param imagePath Ruta de la imagen a cargar.
     * @param width Ancho deseado para la imagen (por defecto 100).
     * @param height Altura deseada para la imagen (por defecto 100).
     *
     * Este método carga la imagen desde la ruta especificada, la escala
     * manteniendo la relación de aspecto y establece el tamaño fijo del widget.
     */
    void setImage(const QString &imagePath, int width = 100, int height = 100);
    void setPixmapImg(const QPixmap &pixmap, int width, int height);

    void setBadgeCount(int count);

    // Fuerza la vibración
    void shake();

signals:
    /**
     * @brief Señal emitida cuando el icono es clickeado.
     *
     * Esta señal se emite al detectar un clic con el botón izquierdo del mouse
     * sobre el widget.
     */
    void clicked();

protected:
    /**
     * @brief Evento que detecta la presión del botón del mouse.
     * @param event Objeto QMouseEvent que contiene la información del clic.
     *
     * Este método detecta si se ha hecho clic con el botón izquierdo del mouse.
     * En ese caso, emite la señal clicked().
     */
    void mousePressEvent(QMouseEvent *event) override;

    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;


private:
    QPixmap originalPixmap;
    bool hoverEnabled;
    int baseWidth{0};
    int baseHeight{0};
    QLabel           *badgeLabel;
    QPropertyAnimation *shakeAnim;
    int               badgeCount = 0;
};

#endif // ICON_H
