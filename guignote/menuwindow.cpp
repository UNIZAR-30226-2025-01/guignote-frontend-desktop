#include "menuwindow.h"
#include "ui_menuwindow.h"
#include "imagebutton.h"
#include <QPixmap>
#include <QTransform>
#include <QPushButton>
#include <QMainWindow>

// Constructor de la clase menu
MenuWindow::MenuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWindow),  // Inicialización de la UI
    boton1v1(nullptr),
    boton2v2(nullptr)
{
    ui->setupUi(this);  // Cargar el diseño de menu.ui

    // ------------- TAPETE FONDO -------------

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

    // Crear los botones
    boton1v1 = new ImageButton(":/images/cartaBoton.png", "1v1", this);
    boton2v2 = new ImageButton(":/images/cartasBoton.png", "2v2", this);

    // ------------- TEMPORAL -------------

    // Conectar los botones a funciones (cuando se hace clic)
    connect(boton1v1, &ImageButton::clicked, this, []() {
        qDebug() << "Botón 1v1 presionado";
    });

    connect(boton2v2, &ImageButton::clicked, this, []() {
        qDebug() << "Botón 2v2 presionado";
    });

    // ------------- BARRA SUPERIOR -------------

    // Crear barra superior
    topBar = new QLabel(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("background-color: #171718;");
    topBar->setGeometry(0, 0, this->width(), 40);

    // ------------- ORNAMENTOS ESQUINAS -------------

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

    repositionOrnaments();
}

// Función para reubicar los ornamentos en la pantalla
void MenuWindow::repositionOrnaments() {
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

void MenuWindow::resizeEvent(QResizeEvent *event) {
    topBar->setGeometry(0, 0, this->width(), 40); // Ajustar barra superior al redimensionar
    repositionOrnaments();

    // Definir tamaños de ventana para interpolación
    int minWindowWidth = 400;
    int maxWindowWidth = 1920;

    // Calcular separación dinámicamente
    int minSeparation = 10;
    int maxSeparation = 75;
    int separacion = minSeparation + (maxSeparation - minSeparation) *
                                         ((this->width() - minWindowWidth) / (float)(maxWindowWidth - minWindowWidth));
    separacion = std::max(minSeparation, std::min(separacion, maxSeparation));

    // Definir tamaños mínimo y máximo para las cartas
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

    // Recolocamos los botones
    int totalWidth = 2 * imageWidth + separacion;
    int startX = (this->width() - totalWidth) / 2;
    int centerY = this->height() / 2 - imageHeight / 2;

    boton1v1->setGeometry(startX, centerY, imageWidth, imageHeight);
    boton2v2->setGeometry(startX + imageWidth + separacion, centerY, imageWidth, imageHeight);

    QWidget::resizeEvent(event);
}



// Destructor de la clase menu
MenuWindow::~MenuWindow() {
    delete ui;  // Solo eliminar si ui fue inicializado
}
