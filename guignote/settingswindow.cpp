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

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
{
    // Configuración de la ventana: sin marco, fondo oscuro, esquinas redondeadas.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 600);

    // Layout principal horizontal.
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // --- Barra lateral ---
    sidebar = new QListWidget(this);
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet(
        "QListWidget { background-color: #2d2d2d; color: #ffffff; border: none; }"
        "QListWidget::item { padding: 15px; }"
        "QListWidget::item:selected { background-color: #575757; }"
        );
    sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    sidebar->addItem("Gráficos");
    mainLayout->addWidget(sidebar);

    // --- Área de ajustes (stacked widget) ---
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    // Página "Gráficos"
    QWidget *graphicsPage = new QWidget(this);
    QVBoxLayout *graphicsLayout = new QVBoxLayout(graphicsPage);
    graphicsLayout->setContentsMargins(20, 20, 20, 20);
    graphicsLayout->setSpacing(20);

    // Botón para cerrar la ventana (esquina superior derecha)
    closeButton = new QPushButton(graphicsPage);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    // Layout superior para alinear el botón de cierre
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(closeButton);
    graphicsLayout->addLayout(topLayout);

    // Título de la página
    QLabel *titleLabel = new QLabel("Ajustes Gráficos", graphicsPage);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    graphicsLayout->addWidget(titleLabel);

    // Grupo para modo de visualización
    QGroupBox *displayGroup = new QGroupBox("Modo de visualización", graphicsPage);
    displayGroup->setStyleSheet(
        "QGroupBox { color: #ffffff; font-size: 18px; border: 1px solid #575757; border-radius: 10px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );
    QHBoxLayout *displayLayout = new QHBoxLayout(displayGroup);
    displayLayout->setSpacing(20);
    displayLayout->setContentsMargins(20, 30, 20, 30);

    // Botones de radio
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

    // Espacio flexible al final
    graphicsLayout->addStretch();

    // Añadir la página "Gráficos" al stackedWidget
    stackedWidget->addWidget(graphicsPage);

    // Seleccionar por defecto la primera opción de la barra lateral (Gráficos)
    sidebar->setCurrentRow(0);

    // Conexión del botón de cerrar para terminar la ventana
    connect(closeButton, &QPushButton::clicked, this, &SettingsWindow::close);

    // Conectar los radios a un slot que haga el cambio de modo
    connect(radioWindowed,   &QRadioButton::clicked, this, &SettingsWindow::updateGraphicsMode);
    connect(radioFullscreen, &QRadioButton::clicked, this, &SettingsWindow::updateGraphicsMode);
}

SettingsWindow::~SettingsWindow()
{
}

void SettingsWindow::updateGraphicsMode()
{

    if (!parentWidget()) {
        return; // Evita problemas si no hay padre.
    }

    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget());
    if (!mainWindow) {
        return; // Si el padre no es un QMainWindow, salimos.
    }

    if (radioFullscreen->isChecked()) {
        mainWindow->showFullScreen();
    } else {
        // showNormal() vuelve al modo ventana
        mainWindow->showNormal();
    }
}
