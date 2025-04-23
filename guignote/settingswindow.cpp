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
#include "menuwindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDebug>
#include <QCloseEvent>
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
SettingsWindow::SettingsWindow(QWidget *mainWindow, QWidget *parent, QString usr)
    : QDialog(parent), mainWindowRef(mainWindow)
{
    this->usr = usr;

    // Ventana sin marco, fondo oscuro, esquinas redondeadas.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 375);

    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Contenedor del encabezado
    QWidget *headerWidget = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel("Ajustes de Sonido", this);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);

    // Slider de volumen
    audioSlider = new QSlider(Qt::Horizontal, this);
    audioSlider->setRange(0, 100);
    audioSlider->setValue(50);
    audioSlider->setFixedHeight(30);
    audioSlider->setStyleSheet(
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

    // Añadir al layout principal
    mainLayout->addWidget(headerWidget);

    // Etiqueta para música
    QLabel *musicLabel = new QLabel("Música", this);
    musicLabel->setStyleSheet("color: #ffffff; font-size: 20px;");
    mainLayout->addWidget(musicLabel);
    mainLayout->addWidget(audioSlider);

    // Etiqueta para sonido
    QLabel *soundLabel = new QLabel("Sonido", this);
    soundLabel->setStyleSheet("color: #ffffff; font-size: 20px;");
    mainLayout->addWidget(soundLabel);

    // soundSlider ya está declarado en el .h
    soundSlider = new QSlider(Qt::Horizontal, this);
    soundSlider->setRange(0, 100);
    soundSlider->setValue(50);
    soundSlider->setFixedHeight(30);
    soundSlider->setStyleSheet(
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
    mainLayout->addWidget(soundSlider);

    // Conectar el botón de cerrar
    connect(closeButton, &QPushButton::clicked, this, &SettingsWindow::close);

    // Conectar con MenuWindow si es posible
    MenuWindow *menuWin = qobject_cast<MenuWindow*>(mainWindowRef);
    if (menuWin) {
        connect(audioSlider, &QSlider::valueChanged,
                menuWin,       &MenuWindow::setVolume);
        if (menuWin->audioOutput) {
            int initialVol = static_cast<int>(menuWin->audioOutput->volume() * 100);
            audioSlider->setValue(initialVol);
        }
    } else {
        qWarning() << "mainWindowRef no es un MenuWindow. No se conectará el slider de volumen.";
    }

    loadSettings();
}


/**
 * @brief Destructor de SettingsWindow.
 *
 * Libera los recursos utilizados por la ventana de configuración. Los widgets hijos son eliminados automáticamente.
 */
SettingsWindow::~SettingsWindow()
{
}

void SettingsWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();  // Guardar los ajustes antes de cerrar la ventana
    event->accept(); // Aceptar el evento de cierre
    emit finished(0);
}


void SettingsWindow::saveSettings()
{
    QString config = "Sota, Caballo y Rey_" + usr;
    QSettings settings("Grace Hopper", config);
    settings.setValue("sound/volume", audioSlider->value());
    settings.setValue("sound/effectsVolume", soundSlider->value());
}

void SettingsWindow::loadSettings()
{
    QString config = "Sota, Caballo y Rey_" + usr;
    QSettings settings("Grace Hopper", config);
    // Cargar volumen y aplicarlo a la barra de sonido
    int volume = settings.value("sound/volume", 50).toInt();  // Valor por defecto: 50
    audioSlider->setValue(volume);
    int effectsVolume = settings.value("sound/effectsVolume", 50).toInt();
    soundSlider->setValue(effectsVolume);
}

