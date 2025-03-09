#include "inventorywindow.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

InventoryWindow::InventoryWindow(QWidget *parent)
    : QDialog(parent)
{
    // Ventana sin marco, esquinas redondeadas
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px;");
    setFixedSize(800, 600);

    // --- 1) Layout vertical contenedor principal ---
    QVBoxLayout *containerLayout = new QVBoxLayout;
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // --- 2) Layout horizontal para la fila superior con la cruz ---
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(10, 10, 10, 0);
        // Ajusta estos márgenes si quieres más/menos espacio en el borde superior
    topLayout->setSpacing(0);

    // Botón de cierre (cruz)
    closeButton = new QPushButton;
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &InventoryWindow::close);

    // Empuja la cruz a la derecha
    topLayout->addStretch();
    topLayout->addWidget(closeButton);

    containerLayout->addLayout(topLayout);

    // --- 3) Layout horizontal principal (sidebar + stackedWidget) ---
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(10, 0, 10, 10);
        // Ajusta márgenes laterales (10) y el inferior (10)
    mainLayout->setSpacing(10);

    // SIDEBAR
    sidebar = new QListWidget;
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet(
        "QListWidget { background-color: #2d2d2d; color: #ffffff; border: none; }"
        "QListWidget::item { padding: 15px; }"
        "QListWidget::item:selected { background-color: #575757; }"
        );
    sidebar->addItem("Barajas");
    sidebar->addItem("Tapetes");
    mainLayout->addWidget(sidebar);

    // STACKED WIDGET
    stackedWidget = new QStackedWidget;
    mainLayout->addWidget(stackedWidget);

    // --- Página 1: Barajas ---
    QWidget *deckPage = new QWidget;
    QVBoxLayout *deckLayout = new QVBoxLayout(deckPage);
    deckLayout->setContentsMargins(0, 0, 0, 0);
    deckLayout->setAlignment(Qt::AlignTop);

    QLabel *deckLabel = new QLabel("Gestión de Barajas");
    deckLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold;");
    deckLabel->setAlignment(Qt::AlignCenter);
    deckLayout->addWidget(deckLabel);

    // Dummys
    QGridLayout *deckPlaceholderLayout = new QGridLayout;
    deckPlaceholderLayout->setSpacing(20);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            QFrame *placeholder = new QFrame;
            placeholder->setFixedSize(150, 100);
            placeholder->setStyleSheet("border: 2px solid white; border-radius: 5px; background-color: transparent;");
            deckPlaceholderLayout->addWidget(placeholder, i, j, Qt::AlignCenter);
        }
    }
    deckLayout->addLayout(deckPlaceholderLayout);
    stackedWidget->addWidget(deckPage);

    // --- Página 2: Tapetes ---
    QWidget *matPage = new QWidget;
    QVBoxLayout *matLayout = new QVBoxLayout(matPage);
    matLayout->setContentsMargins(0, 0, 0, 0);
    matLayout->setAlignment(Qt::AlignTop);

    QLabel *matLabel = new QLabel("Gestión de Tapetes");
    matLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold;");
    matLabel->setAlignment(Qt::AlignCenter);
    matLayout->addWidget(matLabel);

    // Dummys
    QGridLayout *matPlaceholderLayout = new QGridLayout;
    matPlaceholderLayout->setSpacing(20);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            QFrame *placeholder = new QFrame;
            placeholder->setFixedSize(150, 100);
            placeholder->setStyleSheet("border: 2px solid white; border-radius: 5px; background-color: transparent;");
            matPlaceholderLayout->addWidget(placeholder, i, j, Qt::AlignCenter);
        }
    }
    matLayout->addLayout(matPlaceholderLayout);
    stackedWidget->addWidget(matPage);

    // Seleccionar por defecto la sección de Barajas
    sidebar->setCurrentRow(0);
    connect(sidebar, &QListWidget::currentRowChanged,
            stackedWidget, &QStackedWidget::setCurrentIndex);

    // Agregar el layout principal (sidebar + stack) al contenedor
    containerLayout->addLayout(mainLayout);
    setLayout(containerLayout);
}


InventoryWindow::~InventoryWindow()
{
}
