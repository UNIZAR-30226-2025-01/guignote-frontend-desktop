#include "menuwindow.h"
#include "ui_menuwindow.h"
#include "imagebutton.h"

// Constructor de la clase menu
MenuWindow::MenuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWindow),  // Inicialización de la UI
    boton1v1(nullptr),
    boton2v2(nullptr),
    bottomBar(nullptr),
    topBar(nullptr),
    settings(nullptr),
    friends(nullptr),
    divider(nullptr)
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

    // ------------- EVENTOS DE CLICK CARTAS -------------

    // Conectar los botones a funciones (cuando se hace clic)
    connect(boton1v1, &ImageButton::clicked, this, []() {
        qDebug() << "Botón 1v1 presionado";
    });

    connect(boton2v2, &ImageButton::clicked, this, []() {
        qDebug() << "Botón 2v2 presionado";
    });

    // ------------- BARS -------------

    bottomBar = new QFrame(this);
    bottomBar->setStyleSheet("background-color: #171718; border-radius: 10px;");
    topBar = new QFrame(this);
    topBar->setStyleSheet("background-color: #171718; border-radius: 10px;");

    // ------------- SETTINGS Y FRIENDS -------------

    settings = new Icon(this);
    friends = new Icon(this);
    divider = new Icon(this);

    settings->setImage(":/icons/settings.png", 60, 60);
    friends->setImage(":/icons/friends.png", 60, 60);
    divider->setImage(":/icons/divider.png", 60, 60);

    // ------------- EVENTOS DE CLICK SETTINGS Y FRIENDS -------------

    // Conectar señales de clic a funciones
    connect(settings, &Icon::clicked, this, []() {
        qDebug() << "¡Botón de Configuración clickeado!";
    });

    connect(friends, &Icon::clicked, this, []() {
        qDebug() << "¡Botón de Amigos clickeado!";
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

// Función para reposicionar los ImageButtons
void MenuWindow::repositionImageButtons() {

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
}

// Función para reposicionar las barras
void MenuWindow::repositionBars() {
    int w = this->width();
    int barWidthTop = w / 3;  // 1/3 del ancho de la ventana
    int barWidthBottom = w / 4;  // 1/3 del ancho de la ventana
    int barHeight = 80;    // Altura fija de la barra
    int xPosT = (w - barWidthTop) / 2; // Centrado horizontalmente
    int xPosB = (w - barWidthBottom) / 2; // Centrado horizontalmente
    int yPos = this->height() - barHeight; // Pegado abajo con margen de 10px

    topBar->setGeometry(xPosT, 0, barWidthTop, barHeight);
    bottomBar->setGeometry(xPosB, yPos, barWidthBottom, barHeight);
}

void MenuWindow::repositionIcons() {
    // Obtener nuevas dimensiones de la ventana
    int windowWidth = this->width();
    int windowHeight = this->height();

    // Definir el tamaño de las imágenes
    int imgWidth = settings->width();
    int imgHeight = settings->height();
    int dividerWidth = divider->width();

    int separacion = 120;  // Espaciado entre iconos
    int margen = 40;       // Margen desde la parte inferior

    // Posicionar en la parte inferior de la pantalla **centrando las imágenes**
    settings->move((windowWidth / 2) - (imgWidth / 2) - separacion, windowHeight - (imgHeight / 2) - margen);
    friends->move((windowWidth / 2) + separacion - (imgWidth / 2), windowHeight - (imgHeight / 2) - margen);
    divider->move((windowWidth / 2) - (dividerWidth / 2), windowHeight - (imgHeight / 2) - margen);
}


// Función para recolocar y reposicionar todos los elementos
void MenuWindow::resizeEvent(QResizeEvent *event) {
    repositionOrnaments();
    repositionBars();
    repositionImageButtons();
    repositionIcons();

    QWidget::resizeEvent(event);
}

// Destructor de la clase menu
MenuWindow::~MenuWindow() {
    delete ui;  // Solo eliminar si ui fue inicializado
}
