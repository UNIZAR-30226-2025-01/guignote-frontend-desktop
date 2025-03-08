#include "myprofilewindow.h"
#include <QHBoxLayout>

MyProfileWindow::MyProfileWindow(QWidget *parent) : QDialog(parent) {
    // Configura la ventana sin bordes y con estilo personalizado.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 600);

    // Construye la interfaz de usuario.
    setupUI();
}

void MyProfileWindow::setupUI() {
    // Layout principal vertical de la ventana.
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);  // Espaciado entre elementos
    mainLayout->setAlignment(Qt::AlignTop); // Alinea todos los elementos en la parte superior

    // Layout horizontal para el encabezado que contiene el título y el botón de cierre.
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Creación del título y configuración de su estilo.
    titleLabel = new QLabel("Perfil", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Alinea a la izquierda y centra verticalmente
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    // Creación del botón de cierre y configuración de su estilo.
    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    // Conecta la acción de clic para cerrar la ventana.
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    // Agrega el título, un espacio flexible y el botón de cierre al layout del encabezado.
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();  // Empuja el botón de cierre hacia la derecha
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // Agrega el layout del encabezado al layout principal.
    mainLayout->addLayout(headerLayout);

    // Asigna el layout principal a la ventana.
    setLayout(mainLayout);
}
