#include "menu.h"
#include "ui_menu.h"
#include <QPixmap>
#include <QTransform>
#include <QPushButton>

// Constructor de la clase menu
menu::menu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::menu)  // Inicialización de la UI
{
    ui->setupUi(this);  // Cargar el diseño de menu.ui

    // Configuración del fondo con gradiente y barra superior
    this->setStyleSheet("QWidget {"
                        "background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
                        "fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
                        "}"
                        "QWidget#topBar {"
                        "background-color: #171718;"
                        "height: 40px;"
                        "}");

    // ------------- IMAGENES CARTAS -------------

    // Crear QLabel para la imagen de las cartas
    cardLabel = new QLabel(this);

    // Cargar la imagen desde los recursos
    QPixmap cardPixmap(":/images/card_backs.png");

    // Asegurar que el pixmap usa canal alfa (transparencia)
    cardPixmap = cardPixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cardLabel->setPixmap(cardPixmap);

    // Hacer que QLabel sea transparente
    cardLabel->setAttribute(Qt::WA_TranslucentBackground);
    cardLabel->setStyleSheet("background: transparent;");

    // Definir el tamaño del QLabel según la imagen
    cardLabel->setFixedSize(cardPixmap.size());

    // ------------------------------------------

    // Crear barra superior
    topBar = new QLabel(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("background-color: #171718;");
    topBar->setGeometry(0, 0, this->width(), 40);

    // Tamaño de los ornamentos
    ornamentSize = QSize(300, 299);
    QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");

    // Crear y posicionar las esquinas decorativas
    cornerTopLeft = new QLabel(this);
    cornerTopRight = new QLabel(this);
    cornerBottomLeft = new QLabel(this);
    cornerBottomRight = new QLabel(this);

    // Asignar imágenes transformadas para las esquinas
    cornerTopLeft->setPixmap(ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QTransform transformH;
    transformH.scale(-1, 1);
    cornerTopRight->setPixmap(ornamentPixmap.transformed(transformH, Qt::SmoothTransformation)
                                  .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QTransform transformV;
    transformV.scale(1, -1);
    cornerBottomLeft->setPixmap(ornamentPixmap.transformed(transformV, Qt::SmoothTransformation)
                                    .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QTransform transformHV;
    transformHV.scale(-1, -1);
    cornerBottomRight->setPixmap(ornamentPixmap.transformed(transformHV, Qt::SmoothTransformation)
                                     .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Ajustar transparencia y estilos de las esquinas
    QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
    for (QLabel* corner : corners) {
        corner->setFixedSize(ornamentSize);
        corner->setAttribute(Qt::WA_TransparentForMouseEvents);
        corner->setAttribute(Qt::WA_TranslucentBackground);
        corner->setStyleSheet("background: transparent;");
        corner->raise();
    }

    // Crear botones dorados
    button1v1 = new QPushButton("Individual", this);
    button2v2 = new QPushButton("Parejas", this);

    // Estilo para los botones
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

    button2v2->setStyleSheet(button1v1->styleSheet()); // Aplicar el mismo estilo

    // Ajustar posiciones de los botones centrados en la zona verde
    int buttonWidth = 150;
    int buttonHeight = 50;
    int centerX1 = this->width() / 2 - buttonWidth - 10;
    int centerX2 = this->width() / 2 + 10;
    int centerY = (this->height() - 80) / 2 + 80 - buttonHeight / 2;

    button1v1->setGeometry(centerX1, centerY, buttonWidth, buttonHeight);
    button2v2->setGeometry(centerX2, centerY, buttonWidth, buttonHeight);

    repositionOrnaments();
}

// Función para reubicar los ornamentos en la pantalla
void menu::repositionOrnaments() {
    int w = this->width();
    int h = this->height();

    int topOffset = 80;  // Desplazar las esquinas superiores por la barra

    cornerTopLeft->move(0, topOffset);
    cornerTopRight->move(w - cornerTopRight->width(), topOffset);
    cornerBottomLeft->move(0, h - cornerBottomLeft->height());
    cornerBottomRight->move(w - cornerBottomRight->width(), h - cornerBottomRight->height());
    QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
    for (QLabel* corner : corners) {
        corner->lower();
    }
}

void menu::resizeEvent(QResizeEvent *event) {
    topBar->setGeometry(0, 0, this->width(), 40); // Ajustar barra superior al redimensionar
    repositionOrnaments();

    // Definir tamaños mínimo y máximo
    int minButtonWidth = 150;
    int minButtonHeight = 50;
    int maxButtonWidth = 600;
    int maxButtonHeight = 200;

    // Definir tamaños de ventana para interpolación
    int minWindowWidth = 400;
    int maxWindowWidth = 1920;

    // Calcular ancho y alto proporcionalmente entre los valores mínimo y máximo
    int buttonWidth = minButtonWidth + (maxButtonWidth - minButtonWidth) *
                                           ((this->width() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));

    int buttonHeight = minButtonHeight + (maxButtonHeight - minButtonHeight) *
                                             ((this->height() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));

    // Asegurar que los valores no sean menores que el mínimo o mayores que el máximo
    buttonWidth = std::max(minButtonWidth, std::min(buttonWidth, maxButtonWidth));
    buttonHeight = std::max(minButtonHeight, std::min(buttonHeight, maxButtonHeight));

    // Calcular separación dinámicamente
    int minSeparation = 10;
    int maxSeparation = 75;
    int separacion = minSeparation + (maxSeparation - minSeparation) *
                                         ((this->width() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));
    separacion = std::max(minSeparation, std::min(separacion, maxSeparation));

    // Ajustar la posición de los botones para que siempre estén centrados
    int centerX1 = this->width() / 2 - buttonWidth - separacion;
    int centerX2 = this->width() / 2 + separacion;
    int centerY = (this->height() - 80) / 2 + 80 - buttonHeight / 2;

    button1v1->setGeometry(centerX1, centerY, buttonWidth, buttonHeight);
    button2v2->setGeometry(centerX2, centerY, buttonWidth, buttonHeight);

    // Definir tamaños mínimo y máximo para la imagen
    int minImageWidth = 150;
    int minImageHeight = 225;
    int maxImageWidth = 600;
    int maxImageHeight = 900;

    // Calcular tamaño proporcionalmente al tamaño de la ventana
    int imageWidth = minImageWidth + (maxImageWidth - minImageWidth) *
                                         ((this->width() - 400) / (float)(1920 - 400));
    int imageHeight = minImageHeight + (maxImageHeight - minImageHeight) *
                                           ((this->height() - 400) / (float)(1080 - 400));

    // Asegurar que los valores no sean menores que el mínimo o mayores que el máximo
    imageWidth = std::max(minImageWidth, std::min(imageWidth, maxImageWidth));
    imageHeight = std::max(minImageHeight, std::min(imageHeight, maxImageHeight));

    // Cargar y escalar la imagen
    QPixmap cardPixmap(":/images/card_backs.png");
    cardPixmap = cardPixmap.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cardLabel->setPixmap(cardPixmap);
    cardLabel->setFixedSize(cardPixmap.size());

    // Reposicionar la imagen en el centro de la pantalla
    int posX = this->width() / 2 - cardLabel->width() / 2;
    int posY = 85;
    cardLabel->move(posX, posY);


    QWidget::resizeEvent(event);
}



// Destructor de la clase menu
menu::~menu() {
    delete ui;  // Solo eliminar si ui fue inicializado
}
