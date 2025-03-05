#include "friendswindow.h"

#include <QHBoxLayout>

friendswindow::friendswindow(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 600);

    setupUI();  // Llamamos a la función para construir la UI
}

void friendswindow::setupUI() {
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);  // Espaciado ajustado
    mainLayout->setAlignment(Qt::AlignTop); // Alinear todos los elementos arriba

    // Layout superior para el botón de cierre y el título en la misma línea
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Título alineado a la izquierda
    titleLabel = new QLabel("Menú de Amigos", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Alineado verticalmente con el botón
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    // Botón de cierre alineado a la derecha
    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    // Agregar los elementos al header
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();  // Empuja el botón de cierre a la derecha
    headerLayout->addWidget(closeButton);

    mainLayout->addLayout(headerLayout); // Agregar la cabecera al layout principal

    // Crear la barra de búsqueda alineada arriba
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Buscar amigos...");
    searchBar->setStyleSheet(
        "QLineEdit { background-color: #222; color: white; border: 1px solid #555; border-radius: 15px; padding: 8px; font-size: 16px; }"
        "QLineEdit:focus { border: 1px solid #888; }"
        );
    searchBar->setFixedHeight(35);

    mainLayout->addWidget(searchBar, 0, Qt::AlignTop); // Agregar la barra de búsqueda

    setLayout(mainLayout);
}
