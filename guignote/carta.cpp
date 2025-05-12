/**
 * @file carta.cpp
 * @brief Implementación de la clase Carta.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la lógica para mostrar, manipular y detectar interacciones
 * con cartas gráficas dentro de la interfaz del juego.
 */

#include "carta.h"
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsOpacityEffect>

/**
 * @brief Constructor por defecto.
 *
 * Inicializa una carta de reverso visible, sin valor ni palo específico.
 * Utiliza el constructor completo.
 * @param parent Widget padre. Opcional.
 */
Carta::Carta(QWidget* parent)
    : Carta("Back", "", parent) {}

/**
 * @brief Constructor con parámetros de palo y valor.
 *
 * Inicializa una carta con imagen específica y orientación por defecto (abajo).
 * @param palo Palo de la carta.
 * @param valor Valor de la carta.
 * @param parent Widget padre. Opcional.
 */
Carta::Carta(const QString& palo, const QString& valor, QWidget *parent)
    : palo(palo), valor(valor), orientacion(Orientacion::DOWN), QLabel(parent) {
    cargarImagen();
}

/**
 * @brief Establece la posición de la carta en el widget.
 * @param x Coordenada horizontal.
 * @param y Coordenada vertical.
 */
void Carta::setPosicion(int x, int y) {
    this->posX = x;
    this->posY = y;
    this->move(posX, posY);
}

/**
 * @brief Establece el palo y el valor de la carta, recargando la imagen.
 * @param palo Nuevo palo.
 * @param valor Nuevo valor.
 */
void Carta::setPaloValor(const QString& palo, const QString& valor) {
    this->palo = palo;
    this->valor = valor;
    cargarImagen();
    setOrientacion(this->orientacion);
    this->show();
}

/**
 * @brief Devuelve el palo de la carta.
 * @return QString con el palo actual.
 */
QString Carta::getPalo() const {
    return palo;
}

/**
 * @brief Devuelve el valor de la carta.
 * @return QString con el valor actual.
 */
QString Carta::getValor() const {
    return valor;
}

/**
 * @brief Establece la orientación visual de la carta.
 * @param orientacion Nueva orientación.
 */
void Carta::setOrientacion(Orientacion orientacion) {
    this->orientacion = orientacion;
    if (!img[0].isNull()) {
        this->setPixmap(img[orientacion % 2]);
        this->resize(img[orientacion % 2].size());
    }
}


/**
 * @brief Carga la imagen de la carta según su palo, valor y estilo (skin).
 *
 * Se aplican transformaciones para obtener imágenes verticales y horizontales.
 */
void Carta::cargarImagen() {
    QString _skin = "base";
    if (Carta::skin == 1) _skin = "poker";
    if (Carta::skin == 2) _skin = "paint";

    QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();

    img[0] = QPixmap(":/decks/" + _skin + "/" + valor + palo + ".png")
                 .scaled(screenSize.width() * .05f, screenSize.height() * .1f, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QTransform transform;
    transform.rotate(90);
    img[1] = img[0].transformed(transform);

    this->setPixmap(img[orientacion % 2]);
    this->resize(img[orientacion % 2].size());
}

/**
 * @brief Establece el estilo (skin) de la carta y recarga la imagen.
 * @param skinId Identificador del skin (0: base, 1: poker, etc.)
 */
void Carta::setSkin(int skinId) {
    this->skin = skinId;
    cargarImagen();
}

/**
 * @brief Evento al pasar el ratón por encima de la carta.
 *
 * Aplica un efecto de opacidad si la carta es interactuable.
 * @param event Evento de entrada.
 */
void Carta::enterEvent(QEnterEvent* event) {
    if (!interactuable) return;
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.33);
    this->setGraphicsEffect(effect);
    QLabel::enterEvent(event);
}

/**
 * @brief Evento al quitar el cursor de la carta.
 *
 * Elimina el efecto de opacidad si la carta es interactuable.
 * @param event Evento de salida.
 */
void Carta::leaveEvent(QEvent* event) {
    if (!interactuable) return;
    this->setGraphicsEffect(nullptr);
    QLabel::leaveEvent(event);
}

/**
 * @brief Evento de doble clic sobre la carta.
 *
 * Si la carta es interactuable, emite la señal correspondiente.
 * @param event Evento del ratón.
 */
void Carta::mouseDoubleClickEvent(QMouseEvent* event) {
    if (interactuable) {
        this->setGraphicsEffect(nullptr);
        emit cartaDobleClick(this);
    }
    QLabel::mouseDoubleClickEvent(event);
}
