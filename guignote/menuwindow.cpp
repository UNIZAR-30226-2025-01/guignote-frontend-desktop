/**
 * @file menuwindow.cpp
 * @brief Implementación de la clase MenuWindow.
 *
 * La clase MenuWindow define la ventana principal del menú de la aplicación.
 * Se configuran el fondo, los botones de selección de modos de juego, los adornos decorativos
 * y se gestionan los redimensionamientos para mantener una disposición coherente de los elementos.
 */

#include "menuwindow.h"
#include "ui_menuwindow.h"
#include <QPixmap>
#include <QTransform>
#include <QPushButton>

// Constructor de la clase MenuWindow
MenuWindow::MenuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWindow)  // Inicialización de la interfaz generada con Qt Designer
{
    ui->setupUi(this);  // Cargar el diseño definido en menu.ui

    // Configuración del fondo con un gradiente radial y la barra superior
    this->setStyleSheet("QWidget {"
                        "background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
                        "fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
                        "}"
                        "QWidget#topBar {"
                        "background-color: #171718;"
                        "height: 40px;"
                        "}");

    // ------------- IMÁGENES DE CARTAS -------------

    // Crear QLabel para mostrar la imagen de las cartas de atrás
    cartasAtras = new QLabel(this);

    // Crear QLabel para mostrar la imagen de las cartas de delante
    cartasDelante = new QLabel(this);

    // Creación y configuración de la barra superior
    topBar = new QLabel(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("background-color: #171718;");
    topBar->setGeometry(0, 0, this->width(), 40);

    // Definir el tamaño de los adornos decorativos
    ornamentSize = QSize(300, 299);
    QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");

    // Creación de los QLabel que contendrán los adornos en cada esquina
    cornerTopLeft = new QLabel(this);
    cornerTopRight = new QLabel(this);
    cornerBottomLeft = new QLabel(this);
    cornerBottomRight = new QLabel(this);

    // Asignación de la imagen para la esquina superior izquierda
    cornerTopLeft->setPixmap(ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Esquina superior derecha: imagen transformada horizontalmente (invertir X)
    QTransform transformH;
    transformH.scale(-1, 1);
    cornerTopRight->setPixmap(ornamentPixmap.transformed(transformH, Qt::SmoothTransformation)
                                  .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Esquina inferior izquierda: imagen transformada verticalmente (invertir Y)
    QTransform transformV;
    transformV.scale(1, -1);
    cornerBottomLeft->setPixmap(ornamentPixmap.transformed(transformV, Qt::SmoothTransformation)
                                    .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Esquina inferior derecha: imagen transformada en ambas direcciones (invertir X e Y)
    QTransform transformHV;
    transformHV.scale(-1, -1);
    cornerBottomRight->setPixmap(ornamentPixmap.transformed(transformHV, Qt::SmoothTransformation)
                                     .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Configurar propiedades comunes de los adornos: tamaño fijo, transparencia y estilo sin fondo
    QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
    for (QLabel* corner : corners) {
        corner->setFixedSize(ornamentSize);
        corner->setAttribute(Qt::WA_TransparentForMouseEvents);
        corner->setAttribute(Qt::WA_TranslucentBackground);
        corner->setStyleSheet("background: transparent;");
        corner->raise();
    }

    // Creación de los botones dorados para seleccionar modos de juego
    button1v1 = new QPushButton("Individual", this);
    button2v2 = new QPushButton("Parejas", this);

    // Establecer el estilo de los botones con gradientes y bordes dorados
    button1v1->setStyleSheet("QPushButton {"
                             "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
                             "stop:0 #FFD700, stop:0.5 #DAA520, stop:1 #B8860B);"
                             "border: 3px solid #B8860B;"
                             "border-radius: 15px;"
                             "padding: 10px;"
                             "font-size: 18px;"
                             "font-weight: bold;"
                             "color: #3E2723;"
                             "}"
                             "QPushButton:hover {"
                             "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
                             "stop:0 #FFD700, stop:0.5 #FFC107, stop:1 #FFA500);"
                             "border: 3px solid #FFD700;"
                             "}");
    // Aplicar el mismo estilo al botón de parejas
    button2v2->setStyleSheet(button1v1->styleSheet());

    // Ajustar la posición de los botones para que se muestren centrados en la zona designada
    int buttonWidth = 150;
    int buttonHeight = 50;
    int centerX1 = this->width() / 2 - buttonWidth - 10;
    int centerX2 = this->width() / 2 + 10;
    int centerY = (this->height() - 80) / 2 + 80 - buttonHeight / 2;
    button1v1->setGeometry(centerX1, centerY, buttonWidth, buttonHeight);
    button2v2->setGeometry(centerX2, centerY, buttonWidth, buttonHeight);

    // Llamada a la función que reposiciona los adornos decorativos según el tamaño actual de la ventana
    repositionOrnaments();
}

/**
 * @brief Reposiciona los adornos decorativos en las esquinas de la ventana.
 *
 * Calcula la posición de cada adorno teniendo en cuenta el tamaño de la ventana y el desplazamiento
 * requerido por la barra superior.
 */
void MenuWindow::repositionOrnaments() {
    int w = this->width();
    int h = this->height();
    int topOffset = 80;  // Desplazamiento para dejar espacio a la barra superior

    cornerTopLeft->move(0, topOffset);
    cornerTopRight->move(w - cornerTopRight->width(), topOffset);
    cornerBottomLeft->move(0, h - cornerBottomLeft->height());
    cornerBottomRight->move(w - cornerBottomRight->width(), h - cornerBottomRight->height());

    // Asegurar que los adornos se muestren en un orden de pila adecuado (más abajo en la jerarquía visual)
    QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
    for (QLabel* corner : corners) {
        corner->lower();
    }
}

/**
 * @brief Evento de redimensionamiento de la ventana.
 *
 * Se invoca automáticamente cuando el tamaño de la ventana cambia. Se encarga de ajustar la geometría
 * de la barra superior, los botones y las imágenes de las cartas, manteniendo un diseño proporcional.
 *
 * @param event Evento de tipo QResizeEvent.
 */
void MenuWindow::resizeEvent(QResizeEvent *event) {
    // Ajustar la barra superior al nuevo ancho de la ventana
    topBar->setGeometry(0, 0, this->width(), 40);
    // Reposicionar los adornos decorativos
    repositionOrnaments();

    // Definir límites para el tamaño de los botones
    int minButtonWidth = 150;
    int minButtonHeight = 50;
    int maxButtonWidth = 600;
    int maxButtonHeight = 200;

    // Límites para la ventana, utilizados para interpolar el tamaño de los botones
    int minWindowWidth = 400;
    int maxWindowWidth = 1920;

    // Calcular el ancho y alto de los botones de forma proporcional al tamaño de la ventana
    int buttonWidth = minButtonWidth + (maxButtonWidth - minButtonWidth) *
                                           ((this->width() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));
    int buttonHeight = minButtonHeight + (maxButtonHeight - minButtonHeight) *
                                             ((this->height() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));

    // Asegurar que el tamaño de los botones se mantenga dentro de los límites establecidos
    buttonWidth = std::max(minButtonWidth, std::min(buttonWidth, maxButtonWidth));
    buttonHeight = std::max(minButtonHeight, std::min(buttonHeight, maxButtonHeight));

    // Calcular la separación entre botones de forma dinámica
    int minSeparation = 10;
    int maxSeparation = 75;
    int separacion = minSeparation + (maxSeparation - minSeparation) *
                                         ((this->width() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));
    separacion = std::max(minSeparation, std::min(separacion, maxSeparation));

    // Reposicionar los botones para que permanezcan centrados
    int centerX1 = this->width() / 2 - buttonWidth - separacion;
    int centerX2 = this->width() / 2 + separacion;
    int centerY = (this->height() - 80) / 2 + 80 - buttonHeight / 2;
    button1v1->setGeometry(centerX1, centerY, buttonWidth, buttonHeight);
    button2v2->setGeometry(centerX2, centerY, buttonWidth, buttonHeight);

    // Definir límites para el tamaño de las imágenes de las cartas
    int minImageWidth = 150;
    int minImageHeight = 225;
    int maxImageWidth = 600;
    int maxImageHeight = 900;

    // Calcular el tamaño proporcional de las imágenes de las cartas
    int imageWidth = minImageWidth + (maxImageWidth - minImageWidth) *
                                         ((this->width() - 400) / (float)(1920 - 400));
    int imageHeight = minImageHeight + (maxImageHeight - minImageHeight) *
                                           ((this->height() - 400) / (float)(1080 - 400));
    imageWidth = std::max(minImageWidth, std::min(imageWidth, maxImageWidth));
    imageHeight = std::max(minImageHeight, std::min(imageHeight, maxImageHeight));

    // Configurar la imagen de las cartas de atrás
    QPixmap cardPixmapAtras(":/images/card_backs_2.png");
    cardPixmapAtras = cardPixmapAtras.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cartasAtras->setPixmap(cardPixmapAtras);
    cartasAtras->setAttribute(Qt::WA_TranslucentBackground);
    cartasAtras->setStyleSheet("background: transparent;");
    cartasAtras->setFixedSize(cardPixmapAtras.size());
    int posX = this->width() / 2 - cartasAtras->width() / 2;
    int posY = 85;
    cartasAtras->move(posX, posY);

    // Configurar la imagen de las cartas de delante
    QPixmap cardPixmapDelante(":/images/card_fronts.png");
    cardPixmapDelante = cardPixmapDelante.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cartasDelante->setPixmap(cardPixmapDelante);
    cartasDelante->setAttribute(Qt::WA_TranslucentBackground);
    cartasDelante->setStyleSheet("background: transparent;");
    cartasDelante->setFixedSize(cardPixmapDelante.size());
    posX = this->width() / 2 - cartasDelante->width() / 2;
    posY = this->height() - cartasDelante->height() - 5;
    cartasDelante->move(posX, posY);

    QWidget::resizeEvent(event);
}

// Destructor de la clase MenuWindow
MenuWindow::~MenuWindow() {
    delete ui;  // Liberar recursos de la interfaz
}
