/**
 * @file settingswindow.cpp
 * @brief Implementación de la clase SettingsWindow.
 *
 * Esta clase define la ventana de configuración de la aplicación, permitiendo al usuario
 * modificar opciones de visualización, como cambiar entre el modo ventana y el modo pantalla completa.
 * La interfaz se compone de un sidebar para la navegación y un área (QStackedWidget) para mostrar la
 * página de "Gráficos", que incluye opciones configurables mediante botones de radio y un botón de cierre.
 */

#include "settingswindow.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDebug>
#include <QMainWindow>

/**
 * @brief Constructor de SettingsWindow.
 *
 * Configura la ventana de configuración: sin marco, fondo oscuro, esquinas redondeadas y tamaño fijo.
 * Se crea un sidebar (QListWidget) para la navegación y un QStackedWidget para mostrar la página de "Gráficos".
 * En la página de "Gráficos" se incluye un botón de cierre, un título, y un grupo de botones de radio para
 * seleccionar el modo de visualización (ventana o pantalla completa).
 *
 * @param parent Puntero al widget padre, usualmente un QMainWindow.
 */
SettingsWindow::SettingsWindow(QWidget *mainWindow, QWidget *parent)
    : QDialog(parent), mainWindowRef(mainWindow)
{
    // Configuración de la ventana: sin marco, fondo oscuro y esquinas redondeadas.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 600);

    // Layout principal horizontal con márgenes y espaciado.
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // --- Configuración del Sidebar ---
    sidebar = new QListWidget(this);
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet(
        "QListWidget { background-color: #2d2d2d; color: #ffffff; border: none; }"
        "QListWidget::item { padding: 15px; }"
        "QListWidget::item:selected { background-color: #575757; }"
        );
    sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebar->addItem("Gráficos");
    sidebar->addItem("Sonido");
    mainLayout->addWidget(sidebar);

    // --- Configuración del área de ajustes (Stacked Widget) ---
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    // --- Página "Gráficos" ---
    QWidget *graphicsPage = new QWidget(this);
    QVBoxLayout *graphicsLayout = new QVBoxLayout(graphicsPage);
    graphicsLayout->setContentsMargins(20, 20, 20, 20);
    graphicsLayout->setSpacing(20);

    // Botón para cerrar la ventana (ubicado en la esquina superior derecha de la página)
    closeButton = new QPushButton(graphicsPage);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    // Layout superior para alinear el botón de cierre a la derecha.
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(closeButton);
    graphicsLayout->addLayout(topLayout);

    // Título de la página "Gráficos"
    QLabel *titleLabel = new QLabel("Ajustes Gráficos", graphicsPage);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    graphicsLayout->addWidget(titleLabel);

    // Grupo para configurar el modo de visualización
    QGroupBox *displayGroup = new QGroupBox("Modo de visualización", graphicsPage);
    displayGroup->setStyleSheet(
        "QGroupBox { color: #ffffff; font-size: 18px; border: 1px solid #575757; border-radius: 10px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
    QHBoxLayout *displayLayout = new QHBoxLayout(displayGroup);
    displayLayout->setSpacing(20);
    displayLayout->setContentsMargins(20, 30, 20, 30);

    // Botones de radio para seleccionar entre modo "Ventana" y "Pantalla Completa"
    radioWindowed = new QRadioButton("Ventana", displayGroup);
    radioFullscreen = new QRadioButton("Pantalla Completa", displayGroup);
    QString radioStyle =
        "QRadioButton { color: #ffffff; font-size: 16px; padding: 10px; "
        "border: 1px solid #575757; border-radius: 10px; min-width: 120px; text-align: center; }"
        "QRadioButton::indicator { width: 20px; height: 20px; }"
        "QRadioButton::indicator:unchecked { border: 1px solid #575757; background-color: transparent; }"
        "QRadioButton::indicator:checked { border: 1px solid #575757; background-color: #c2c2c3; }";
    radioWindowed->setStyleSheet(radioStyle);
    radioFullscreen->setStyleSheet(radioStyle);
    radioWindowed->setChecked(true);

    displayLayout->addWidget(radioWindowed);
    displayLayout->addWidget(radioFullscreen);
    graphicsLayout->addWidget(displayGroup);

    // Espacio flexible para completar el layout.
    graphicsLayout->addStretch();

    // Añadir la página "Gráficos" al stacked widget.
    stackedWidget->addWidget(graphicsPage);

    // Seleccionar por defecto la primera opción del sidebar.
    sidebar->setCurrentRow(0);

    // --- Página "Sonido" ---
    QWidget *soundPage = new QWidget(this);
    QVBoxLayout *soundLayout = new QVBoxLayout(soundPage);
    soundLayout->setContentsMargins(20, 20, 20, 20);
    soundLayout->setSpacing(10); // Se reduce el espacio entre elementos para "subir" la barra

    // Botón para cerrar la ventana en la página "Sonido"
    QPushButton *closeButtonSound = new QPushButton(soundPage);
    closeButtonSound->setIcon(QIcon(":/icons/cross.png"));
    closeButtonSound->setIconSize(QSize(18, 18));
    closeButtonSound->setFixedSize(30, 30);
    closeButtonSound->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    // Layout superior para alinear el botón de cierre a la derecha.
    QHBoxLayout *soundTopLayout = new QHBoxLayout();
    soundTopLayout->addStretch();
    soundTopLayout->addWidget(closeButtonSound);
    soundLayout->addLayout(soundTopLayout);

    // Título de la página "Sonido" (igual que en "Gráficos")
    QLabel *soundTitle = new QLabel("Ajustes de Sonido", soundPage);
    soundTitle->setStyleSheet("color: #ffffff; font-size: 28px; font-weight: bold;");
    soundTitle->setAlignment(Qt::AlignCenter);
    soundLayout->addWidget(soundTitle);

    // Agregar el control de volumen (barra) justo debajo del título, con márgenes reducidos
    QSlider *volumeSlider = new QSlider(Qt::Horizontal, soundPage);
    volumeSlider->setRange(0, 100);      // Rango del volumen de 0 a 100
    volumeSlider->setValue(50);          // Valor inicial al 50%
    volumeSlider->setFixedHeight(30);

    // Estilo personalizado para el slider (groove y handle)
    volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "  border: 1px solid #575757;"
        "  height: 8px;"
        "  background: #222;"
        "  margin: 0 15px;"
        "  border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: #c2c2c3;"
        "  border: 1px solid #575757;"
        "  width: 20px;"
        "  margin: -5px 0;"
        "  border-radius: 10px;"
        "}"
        );
    soundLayout->addWidget(volumeSlider);

    // Espacio flexible para completar el layout.
    soundLayout->addStretch();

    // Añadir la página "Sonido" al stacked widget.
    stackedWidget->addWidget(soundPage);

    // Conexión para cerrar la ventana desde la página "Sonido"
    connect(closeButtonSound, &QPushButton::clicked, this, &SettingsWindow::close);

    // --- Conexión para cambiar de página ---
    // Al cambiar la selección en el sidebar, se muestra la página correspondiente.
    connect(sidebar, &QListWidget::currentRowChanged, this, [=](int row){
        if (row == 0) {
            // Se muestra la página de Gráficos.
            stackedWidget->setCurrentWidget(graphicsPage);
        } else if (row == 1) {
            // Se oculta la página de Gráficos y se muestra la de Sonido.
            stackedWidget->setCurrentWidget(soundPage);
        }
    });

    // Conexión: el botón de cierre cierra la ventana de configuración.
    connect(closeButton, &QPushButton::clicked, this, &SettingsWindow::close);

    // Conexión: al pulsar los botones de radio se actualiza el modo gráfico.
    connect(radioWindowed,   &QRadioButton::clicked, this, &SettingsWindow::updateGraphicsMode);
    connect(radioFullscreen, &QRadioButton::clicked, this, &SettingsWindow::updateGraphicsMode);
}

/**
 * @brief Destructor de SettingsWindow.
 *
 * Libera los recursos utilizados por la ventana de configuración. Los widgets hijos son eliminados automáticamente.
 */
SettingsWindow::~SettingsWindow()
{
}

/**
 * @brief Actualiza el modo gráfico de la aplicación.
 *
 * Este método cambia el modo de visualización del widget padre, que se espera sea un QMainWindow.
 * Si se selecciona el modo "Pantalla Completa", se llama a showFullScreen(); de lo contrario, se llama a showNormal().
 */
void SettingsWindow::updateGraphicsMode()
{
    if (!mainWindowRef) {
        qDebug() << "No se encontró la ventana principal.";
        return;
    }

    if (radioFullscreen->isChecked()) {
        qDebug() << "Cambiando a modo Pantalla Completa.";
        mainWindowRef->showFullScreen();
    } else {
        qDebug() << "Cambiando a modo Ventana.";
        mainWindowRef->showNormal();
    }
}



