#include "myprofilewindow.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include "icon.h"

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

    // ------------- SETTINGS DE VENTANA -------------

    // Layout principal vertical de la ventana.
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);  // Espaciado entre elementos
    mainLayout->setAlignment(Qt::AlignTop); // Alinea los elementos en la parte superior

    // ------------- TITULO Y BOTON DE CIERRE -------------

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

    // *** Centrar la imagen debajo del título y el botón de cierre ***
    QVBoxLayout *imageLayout = new QVBoxLayout();
    imageLayout->setAlignment(Qt::AlignCenter);

    // Espacio flexible arriba de la imagen (para empujarla hacia el centro)
    imageLayout->addStretch();

    // ------------- FOTO PERFIL CIRCULAR -------------

    // Tamaño foto de perfil
    int pfpSize = 200;

    // Ruta de la imagen original
    QString imagePath = ":/icons/profile.png"; // Asegúrate de usar una ruta válida

    // Crear la imagen circular
    QPixmap circularImage = createCircularImage(imagePath, pfpSize);

    // Crear el widget Icon y establecer la imagen
    fotoPerfil = new Icon();
    fotoPerfil->setPixmap(circularImage);
    fotoPerfil->setFixedSize(pfpSize, pfpSize); // Tamaño fijo para la imagen

    connect(fotoPerfil, &Icon::clicked, [=]() {
        qDebug() << "Foto de perfil clickada";
    });

    // Centrar la imagen en el layout secundario
    imageLayout->addWidget(fotoPerfil, 0, Qt::AlignCenter);

    // ------------- MOSTRAR USUARIO Y RANGO -------------

    int ELO = 0;
    QString rank = "Rango";

    // Crear el texto con diferentes estilos
    QString UsrELORank = QString(
                             "<span style='font-size: 24px; font-weight: bold; color: white;'>Usuario (%1)</span><br>"
                             "<span style='font-size: 20px; font-weight: normal; color: white;'>%2</span>"
                             ).arg(ELO).arg(rank);

    // 🔹 Agregar el texto "Usuario" y "Rango" con diferentes estilos 🔹
    QLabel *userLabel = new QLabel(UsrELORank, this);
    userLabel->setAlignment(Qt::AlignCenter);
    userLabel->setTextFormat(Qt::RichText);  // Indicar que usamos HTML
    userLabel->setStyleSheet("color: white;");  // Aplicar color base para coherencia

    imageLayout->addWidget(userLabel);

    // ------------- MOSTRAR ESTADISTICAS -------------

    int Vic = 0;
    int Der = 0;
    int Ratio = 0;
    int Racha = 0;
    int MaxRacha = 0;

    QString stats = "Victorias: " + QString::number(Vic) + "\n"
                    "Derrotas: " + QString::number(Der) + "\n"
                    "Ratio Victoria-Derrota: " + QString::number(Ratio) + "%" + "\n"
                    "Racha Actual: " + QString::number(Racha) + "\n"
                    "Mejor Racha: " + QString::number(MaxRacha) + "\n";

    QLabel *statsLabel = new QLabel(stats, this);
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setStyleSheet("color: white; font-size: 24px;");

    imageLayout->addWidget(statsLabel);

    // ------------- APLICAR LAYOUTS -------------

    // Agregar el layout de la imagen al layout principal
    mainLayout->addLayout(imageLayout);

    // Asigna el layout principal a la ventana.
    setLayout(mainLayout);
}

/**
 * @brief Convierte una imagen en un círculo.
 * @param imagePath Ruta de la imagen a cargar.
 * @param size Tamaño deseado del círculo (ancho y alto serán iguales).
 * @return QPixmap con la imagen recortada en forma circular.
 */
QPixmap MyProfileWindow::createCircularImage(const QString &imagePath, int size) {
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        qDebug() << "Error: No se pudo cargar la imagen desde " << imagePath;
        return QPixmap();
    }

    // Escalar la imagen al tamaño deseado
    pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // Crear un QPixmap circular
    QPixmap circularPixmap(size, size);
    circularPixmap.fill(Qt::transparent);

    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Crear una máscara circular
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    // Dibujar la imagen dentro de la máscara circular
    painter.drawPixmap(0, 0, size, size, pixmap);

    return circularPixmap;
}
