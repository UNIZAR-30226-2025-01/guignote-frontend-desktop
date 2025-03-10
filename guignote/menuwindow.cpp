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
#include "imagebutton.h"
#include "settingswindow.h"
#include "friendswindow.h"
#include "myprofilewindow.h"
//#include "otherprofilewindow.h"

// Constructor de la clase MenuWindow
MenuWindow::MenuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWindow),  // Inicialización de la UI
    boton1v1(nullptr),
    boton2v2(nullptr),
    bottomBar(nullptr),
    topBar(nullptr),
    settings(nullptr),
    friends(nullptr),
    exit(nullptr),
    usrLabel(nullptr)
{
    ui->setupUi(this);  // Cargar el diseño definido en menu.ui

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

    // ------------- IMÁGENES DE CARTAS -------------

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

    // ------------- DETECTAR CLICKS EN TOPBAR -------------

    invisibleButton = new QPushButton(this);  // Botón dentro de topBar
    invisibleButton->setStyleSheet("background: transparent; border: none;");  // Invisible
    invisibleButton->setCursor(Qt::PointingHandCursor);  // Mantiene el cursor de puntero

    connect(invisibleButton, &QPushButton::clicked, [=]() {
        MyProfileWindow *profileWin = new MyProfileWindow(this);
        profileWin->setModal(true);
        profileWin->show();
    });

    // ------------- NOMBRE DE USUARIO Y RANGO EN TOPBAR -------------

    usrLabel = new QLabel(this);  // ✅ Use the global class member

    QString usr = "Usuario";
    int ELO = 0;
    QString rank = "Rango";

    // Create the styled text
    QString UsrELORank = QString(
                             "<span style='font-size: 24px; font-weight: bold; color: white;'>%1 (%2) </span>"
                             "<span style='font-size: 20px; font-weight: normal; color: white;'>%3</span>"
                             ).arg(usr).arg(ELO).arg(rank);

    usrLabel->setText(UsrELORank);
    usrLabel->setAlignment(Qt::AlignCenter);
    usrLabel->setTextFormat(Qt::RichText);
    usrLabel->setStyleSheet("color: white; background: transparent;");

    // ------------- SETTINGS Y FRIENDS -------------

    settings = new Icon(this);
    friends = new Icon(this);
    exit = new Icon(this);

    settings->setImage(":/icons/settings.png", 60, 60);
    friends->setImage(":/icons/friends.png", 60, 60);
    exit->setImage(":/icons/door.png", 60, 60);

    // ------------- EVENTOS DE CLICK SETTINGS Y FRIENDS -------------

    // Conectar señales de clic a funciones
    connect(settings, &Icon::clicked, [=]() {
        SettingsWindow *settingsWin = new SettingsWindow(this);
        settingsWin->setModal(true);
        settingsWin->show();
    });

    connect(friends, &Icon::clicked, this, [this]() {
        // Crear y mostrar la ventana de amigos
        friendswindow *friendsWin = new friendswindow(this);
        friendsWin->setAttribute(Qt::WA_DeleteOnClose); // Hace que se elimine automáticamente al cerrarse
        friendsWin->show();
    });

    connect(exit, &Icon::clicked, this, [this]() {
        this->close();  // Cierra la ventana
    });

    // ------------- ORNAMENTOS ESQUINAS -------------

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

    int topOffset = 0;

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
    QPoint topBarPos = topBar->pos(); // Obtener la posición relativa dentro de la ventana
    invisibleButton->setGeometry(topBarPos.x(), topBarPos.y(), topBar->width(), topBar->height());

    // ✅ Position `usrLabel` in front of `topBar` but behind `invisibleButton`
    int usrLabelHeight = 30;
    int usrLabelWidth = qMin(200, barWidthTop - 20);

    int xUsr = xPosT + (barWidthTop - usrLabelWidth) / 2;  // Center horizontally
    int yUsr = topBarPos.y() + (barHeight - usrLabelHeight) / 2;  // Center vertically inside topBar

    usrLabel->setGeometry(xUsr, yUsr, usrLabelWidth, usrLabelHeight);

    // 🔹 Set stacking order
    topBar->lower();            // Send topBar to back
    usrLabel->raise();          // Bring usrLabel in front of topBar
    invisibleButton->raise();   // Bring invisibleButton in front of usrLabel
}

void MenuWindow::repositionIcons() {
    // Obtener nuevas dimensiones de la ventana
    int windowWidth = this->width();
    int windowHeight = this->height();

    // Definir el tamaño de las imágenes
    int imgWidth = settings->width();
    int imgHeight = settings->height();
    int exitHeight = exit ->height();
    int exitWidth = exit->width();

    int separacion = windowWidth * 0.066;  // Espaciado entre iconos
    int margen = 40;       // Margen desde la parte inferior

    // Posicionar en la parte inferior de la pantalla **centrando las imágenes**
    settings->move((windowWidth / 2) - (imgWidth / 2) - separacion, windowHeight - (imgHeight / 2) - margen);
    friends->move((windowWidth / 2) + separacion - (imgWidth / 2), windowHeight - (imgHeight / 2) - margen);
    exit->move((windowWidth / 2) - (exitWidth / 2), windowHeight - (exitHeight / 2) - margen);
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
    delete ui;  // Liberar recursos de la interfaz
}
