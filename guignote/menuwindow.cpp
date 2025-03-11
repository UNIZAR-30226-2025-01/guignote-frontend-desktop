/**
 * @file menuwindow.cpp
 * @brief Implementación de la clase MenuWindow.
 *
 * La clase MenuWindow define la ventana principal del menú de la aplicación.
 * Se configuran el fondo, los botones de selección de modos de juego, los adornos decorativos
 * y se gestionan los redimensionamientos para mantener una disposición coherente de los elementos.
 */

#include "menuwindow.h"
#include "icon.h"
#include "ui_menuwindow.h"
#include "imagebutton.h"
#include "inventorywindow.h"
#include "settingswindow.h"
#include "friendswindow.h"
#include <qgraphicseffect.h>
#include <QTimer>

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
    exit(nullptr)
{
    ui->setupUi(this);  // Cargar el diseño definido en menu.ui

    // ------------- IMÁGENES DE CARTAS -------------

    // Crear los botones
    boton1v1 = new ImageButton(":/images/cartaBoton.png", "Individual", this);
    boton2v2 = new ImageButton(":/images/cartasBoton.png", "Parejas", this);

    // En el constructor de MenuWindow (menuwindow.cpp):
    backgroundPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);

    // Enlazas el reproductor con la salida de audio
    backgroundPlayer->setAudioOutput(audioOutput);

    // Ahora ajustas el volumen a través de QAudioOutput:
    audioOutput->setVolume(0.5); // Rango de 0.0 a 1.0

    // Asignas el archivo de música (por ejemplo, un recurso)
    backgroundPlayer->setSource(QUrl("qrc:/bgm/menu_jazz_lofi.mp3"));


    // Establecer el número de repeticiones (en Qt 6 en adelante):
    backgroundPlayer->setLoops(QMediaPlayer::Infinite);

    // Reproducir
    backgroundPlayer->play();

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
    topBar = new QFrame(this);

    topBar->setObjectName("topBar");
    bottomBar->setObjectName("bottomBar");

    // ------------- SETTINGS Y FRIENDS -------------

    settings = new Icon(this);
    friends = new Icon(this);
    exit = new Icon(this);
    inventory = new Icon(this);

    settings->setImage(":/icons/settings.png", 60, 60);
    friends->setImage(":/icons/friends.png", 60, 60);
    exit->setImage(":/icons/door.png", 60, 60);
    inventory->setImage(":/icons/chest.png", 60, 60);

    // ------------- EVENTOS DE CLICK SETTINGS Y FRIENDS -------------

    // Ventana de Settings con cuadro modal similar
    connect(settings, &Icon::clicked, [=]() {
        SettingsWindow *settingsWin = new SettingsWindow(this, this);
        settingsWin->setModal(true);
        settingsWin->exec();
    });

    // Ventana de Amigos creada de forma similar a MainWindow
    connect(friends, &Icon::clicked, this, [this]() {
        friendswindow *friendsWin = new friendswindow(this);
        friendsWin->setModal(true);
        friendsWin->exec();
    });

    // Ventana de Confirmar Salir con fondo oscurecido y bloqueo de interacción
    connect(exit, &Icon::clicked, this, [this]() {
        exit->setImage(":/icons/darkeneddoor.png", 60, 60);
        QDialog *confirmDialog = new QDialog(this);
        confirmDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        confirmDialog->setModal(true);
        confirmDialog->setFixedSize(300,150);
        confirmDialog->setStyleSheet(
            "QDialog {"
            "  background-color: #171718;"
            "  border-radius: 5px;"
            "  padding: 20px;"
            "}"
            );
        QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(confirmDialog);
        dialogShadow->setBlurRadius(10);
        dialogShadow->setColor(QColor(0, 0, 0, 80));
        dialogShadow->setOffset(4, 4);
        confirmDialog->setGraphicsEffect(dialogShadow);
        QVBoxLayout *dialogLayout = new QVBoxLayout(confirmDialog);
        QLabel *confirmLabel = new QLabel("¿Está seguro que desea salir?", confirmDialog);
        confirmLabel->setStyleSheet("QFrame { background-color: #171718; color: white; border-radius: 5px; }");
        confirmLabel->setAlignment(Qt::AlignCenter);
        dialogLayout->addWidget(confirmLabel);
        QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
        QString buttonStyle =
            "QPushButton {"
            "  background-color: #c2c2c3;"
            "  color: #171718;"
            "  border-radius: 15px;"
            "  font-size: 20px;"
            "  font-weight: bold;"
            "  padding: 12px 25px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #9b9b9b;"
            "}";
        QPushButton *yesButton = new QPushButton("Sí", confirmDialog);
        QPushButton *noButton = new QPushButton("No", confirmDialog);
        yesButton->setStyleSheet(buttonStyle);
        noButton->setStyleSheet(buttonStyle);
        yesButton->setFixedSize(100,40);
        noButton->setFixedSize(100,40);
        dialogButtonLayout->addWidget(yesButton);
        dialogButtonLayout->addWidget(noButton);
        dialogLayout->addLayout(dialogButtonLayout);
        connect(yesButton, &QPushButton::clicked, [=]() {
            qApp->quit();
        });
        connect(noButton, &QPushButton::clicked, [=]() {
            confirmDialog->close();
        });
        connect(confirmDialog, &QDialog::finished, [=](int) {
            exit->setImage(":/icons/door.png", 60, 60);
        });
        // Posicionar inicialmente en el centro del padre
        confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        confirmDialog->show();

        // QTimer para mantener el diálogo centrado mientras se muestra
        QTimer *centerTimer = new QTimer(confirmDialog);
        centerTimer->setInterval(50); // cada 50 ms
        connect(centerTimer, &QTimer::timeout, [this, confirmDialog]() {
            confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        });
        centerTimer->start();
    });



    // Ventana de Inventory con cuadro modal similar
    connect(inventory, &Icon::clicked, this, [this]() {
        InventoryWindow *inventoryWin = new InventoryWindow(this);
        inventoryWin->setModal(true);
        inventoryWin->exec();
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

    this->setStyleSheet(R"(
    /* Fondo de la ventana con gradiente verde */
    QWidget {
        background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                    fx:0.5, fy:0.5,
                                    stop:0 #1f5a1f,
                                    stop:1 #0a2a08);
    }

    /* Barras top y bottom con gradiente vertical gris–negro */
    QFrame#topBar, QFrame#bottomBar {
        background: qlineargradient(
            spread: pad,
            x1: 0, y1: 0,
            x2: 0, y2: 1,
            stop: 0 #3a3a3a, /* Gris medio */
            stop: 1 #000000 /* Negro */
        );
        border-radius: 8px;
        border: 2px solid #000000; /* Borde negro sólido de 2px */
    }
)");

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

// Función para posicionar los iconos arriba
void MenuWindow::repositionTopIcons() {
    int windowWidth = this->width();
    // Definir el tamaño del icono
    int imgWidth = inventory->width();
    int imgHeight = inventory->height();
    int margen = 12; // Margen desde la parte superior
    // Posicionar en la parte superior centro
    inventory->move((windowWidth / 2) - (imgWidth / 2), margen);
}

// Función para recolocar y reposicionar todos los elementos
void MenuWindow::resizeEvent(QResizeEvent *event) {
    repositionOrnaments();
    repositionBars();
    repositionImageButtons();
    repositionIcons();
    repositionTopIcons();
    QWidget::resizeEvent(event);
}


// Destructor de la clase menu
MenuWindow::~MenuWindow() {
    delete ui;  // Liberar recursos de la interfaz
}
