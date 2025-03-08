#include "friendswindow.h"
#include <QHBoxLayout>

/**
 * @brief Constructor de la ventana de amigos.
 * @param parent Widget padre, por defecto es nullptr.
 *
 * Configura las propiedades generales de la ventana, como el estilo, el tamaño fijo y los flags
 * de la ventana. Luego, llama a setupUI() para construir la interfaz gráfica.
 */
friendswindow::friendswindow(QWidget *parent) : QDialog(parent) {
    // Configura la ventana sin bordes y con estilo personalizado.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 600);

    // Construye la interfaz de usuario.
    setupUI();
}

/**
 * @brief Configura la interfaz de usuario de la ventana de amigos.
 *
 * Este método crea y organiza todos los elementos gráficos:
 * - Un layout principal vertical con márgenes y espaciado ajustados.
 * - Un header (layout horizontal) que contiene el título y el botón de cierre.
 * - Una barra de búsqueda para filtrar los amigos.
 *
 * Finalmente, se asigna el layout principal a la ventana.
 */
void friendswindow::setupUI() {
    // Layout principal vertical de la ventana.
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);  // Espaciado entre elementos
    mainLayout->setAlignment(Qt::AlignTop); // Alinea todos los elementos en la parte superior

    // Layout horizontal para el encabezado que contiene el título y el botón de cierre.
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Creación del título y configuración de su estilo.
    titleLabel = new QLabel("Menú de Amigos", this);
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

    // Creación y configuración de la barra de búsqueda.
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Buscar amigos...");
    searchBar->setStyleSheet(
        "QLineEdit { background-color: #222; color: white; border: 1px solid #555; border-radius: 15px; padding: 8px; font-size: 16px; }"
        "QLineEdit:focus { border: 1px solid #888; }"
        );
    searchBar->setFixedHeight(35);

    // Agrega la barra de búsqueda al layout principal, alineándola en la parte superior.
    mainLayout->addWidget(searchBar, 0, Qt::AlignTop);

    // Asigna el layout principal a la ventana.
    setLayout(mainLayout);
}

QWidget* friendswindow::createFriendsTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    // Lista de amigos
    QListWidget *friendsList = new QListWidget(page);
    friendsList->addItem("Juan Pérez");
    friendsList->addItem("María González");
    friendsList->addItem("Carlos Rodríguez");

    friendsList->setStyleSheet(
        "QListWidget { background-color: #222; color: white; border-radius: 10px; padding: 5px; }"
        );

    layout->addWidget(friendsList);
    page->setLayout(layout);
    return page;
}

QWidget* friendswindow::createRequestsTab() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    // Lista de solicitudes
    QListWidget *requestsList = new QListWidget(page);
    requestsList->addItem("Luis Fernández");
    requestsList->addItem("Ana Martínez");

    requestsList->setStyleSheet(
        "QListWidget { background-color: #222; color: white; border-radius: 10px; padding: 5px; }"
        );

    layout->addWidget(requestsList);

    // Botones de aceptar y rechazar
    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    QPushButton *acceptButton = new QPushButton("Aceptar", page);
    acceptButton->setStyleSheet("background-color: #4CAF50; color: white; border-radius: 10px; padding: 8px;");
    QPushButton *rejectButton = new QPushButton("Rechazar", page);
    rejectButton->setStyleSheet("background-color: #E53935; color: white; border-radius: 10px; padding: 8px;");

    buttonsLayout->addWidget(acceptButton);
    buttonsLayout->addWidget(rejectButton);

    layout->addLayout(buttonsLayout);
    page->setLayout(layout);
    return page;
}
