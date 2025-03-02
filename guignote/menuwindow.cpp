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
    boton1v1 = new ImageButton(":/images/cartaBoton.png", "Individual", this);
    boton2v2 = new ImageButton(":/images/cartasBoton.png", "Parejas", this);

    // ------------- TEMPORAL -------------

    // Conectar los botones a funciones (cuando se hace clic)
    connect(boton1v1, &ImageButton::clicked, this, []() {
        qDebug() << "Botón 1v1 presionado";
    });

    connect(boton2v2, &ImageButton::clicked, this, []() {
        qDebug() << "Botón 2v2 presionado";
    });

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

    int topOffset = 0;

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
    repositionOrnaments();

    int w = this->width();
    int h = this->height();
    int buttonSpacing = w / 10; // Espaciado proporcional al tamaño de la ventana

    // Redimensionar los botones
    QSize size = boton1v1->updatesize(h);
    size = boton2v2->updatesize(h);
    int buttonWidth = size.width();
    int buttonHeight = size.height();

    // Calcular la posición central
    int totalWidth = (2 * buttonWidth) + buttonSpacing;
    int startX = (w - totalWidth) / 2;
    int startY = (h - buttonHeight) / 2;

    boton1v1->move(startX, startY);
    boton2v2->move(startX + buttonWidth + buttonSpacing, startY);

    QWidget::resizeEvent(event);
}

// Destructor de la clase menu
MenuWindow::~MenuWindow() {
    delete ui;  // Solo eliminar si ui fue inicializado
}
