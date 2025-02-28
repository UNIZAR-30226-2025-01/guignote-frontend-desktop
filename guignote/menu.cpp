#include "menu.h"
#include "ui_menu.h"  // ¡IMPORTANTE! Debe estar aquí
#include <QPixmap>
#include <QTransform>

menu::menu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::menu)  // INICIALIZAR UI AQUÍ
{
    ui->setupUi(this);  // Cargar el diseño de `menu.ui`

    // Configuración del fondo con gradiente y barra superior
    this->setStyleSheet("QWidget {"
                        "background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
                        "fx:0.5, fy:0.5, stop:0 #1f5a1f, stop:1 #0a2a08);"
                        "}"
                        "QWidget#topBar {"
                        "background-color: #171718;"
                        "height: 40px;"
                        "}");

    // Crear barra superior
    topBar = new QLabel(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("background-color: #171718;");
    topBar->setGeometry(0, 0, this->width(), 40);

    // Tamaño de los ornamentos
    ornamentSize = QSize(300, 299);
    QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");

    // Crear las esquinas
    cornerTopLeft = new QLabel(this);
    cornerTopRight = new QLabel(this);
    cornerBottomLeft = new QLabel(this);
    cornerBottomRight = new QLabel(this);

    // Asignar imágenes transformadas
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

    // Ajustar transparencia y estilos
    QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
    for (QLabel* corner : corners) {
        corner->setFixedSize(ornamentSize);
        corner->setAttribute(Qt::WA_TransparentForMouseEvents);
        corner->setAttribute(Qt::WA_TranslucentBackground);
        corner->setStyleSheet("background: transparent;");
        corner->raise();
    }

    // Posicionar ornamentos
    repositionOrnaments();
}

void menu::repositionOrnaments() {
    int w = this->width();
    int h = this->height();

    int topOffset = 80;  // Desplazar las esquinas superiores por la barra

    cornerTopLeft->move(0, topOffset);
    cornerTopRight->move(w - cornerTopRight->width(), topOffset);
    cornerBottomLeft->move(0, h - cornerBottomLeft->height());
    cornerBottomRight->move(w - cornerBottomRight->width(), h - cornerBottomRight->height());
}

void menu::resizeEvent(QResizeEvent *event) {
    topBar->setGeometry(0, 0, this->width(), 40); // Ajustar barra superior al redimensionar
    repositionOrnaments();
    QWidget::resizeEvent(event);
}

menu::~menu() {
    delete ui;  // Solo eliminar si `ui` fue inicializado
}
