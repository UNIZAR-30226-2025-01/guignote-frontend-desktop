#include "friendswindow.h"
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>

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
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    // Layout superior para el botón de cierre y el título
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Título alineado a la izquierda
    titleLabel = new QLabel("Menú de Amigos", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // Crear la barra de búsqueda
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Buscar amigos...");
    searchBar->setStyleSheet(
        "QLineEdit { background-color: #222; color: white; border: 1px solid #555; border-radius: 15px; padding: 8px; font-size: 16px; }"
        "QLineEdit:focus { border: 1px solid #888; }"
        );
    searchBar->setFixedHeight(35);
    mainLayout->addWidget(searchBar, 0, Qt::AlignTop);

    // Crear el TabWidget
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #555; border-radius: 10px; }"
        "QTabBar::tab { background: #222; color: white; padding: 10px; border: 1px solid #555; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
        "QTabBar::tab:selected { background: #333; font-weight: bold; }"
        );

    // Crear las páginas de las pestañas
    QWidget *friendsPage = createFriendsTab();
    QWidget *requestsPage = createRequestsTab();

    tabWidget->addTab(friendsPage, "Amigos");
    tabWidget->addTab(requestsPage, "Solicitudes");

    mainLayout->addWidget(tabWidget);
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
