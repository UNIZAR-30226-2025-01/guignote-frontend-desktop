#include "userprofilewindow.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include "icon.h"

UserProfileWindow::UserProfileWindow(QWidget *parent) : QDialog(parent) {
    // Configura la ventana sin bordes y con estilo personalizado.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(800, 600);

    // Construye la interfaz de usuario.
    setupUI();
}

void UserProfileWindow::setupUI() {

    // ------------- SETTINGS DE VENTANA -------------

    // Layout principal vertical de la ventana.
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    // ------------- TITULO Y BOTON DE CIERRE -------------

    QHBoxLayout *headerLayout = new QHBoxLayout();

    titleLabel = new QLabel("Perfil Amigo", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // *** Centrar la imagen debajo del título y el botón de cierre ***
    QVBoxLayout *imageLayout = new QVBoxLayout();
    imageLayout->setAlignment(Qt::AlignCenter);

    imageLayout->addStretch();

    int pfpSize = 200;
    QString imagePath = ":/icons/profile.png";
    QPixmap circularImage = createCircularImage(imagePath, pfpSize);

    fotoPerfil = new Icon();
    fotoPerfil->setHoverEnabled(false);
    fotoPerfil->setPixmap(circularImage);
    fotoPerfil->setFixedSize(pfpSize, pfpSize);

    imageLayout->addWidget(fotoPerfil, 0, Qt::AlignCenter);

    QString usr = "Usuario";
    int ELO = 0;
    QString rank = "Rango";

    QString UsrELORank = QString(
                             "<span style='font-size: 24px; font-weight: bold; color: white;'>%1(%2)</span><br>"
                             "<span style='font-size: 20px; font-weight: normal; color: white;'>%3</span>"
                             ).arg(usr).arg(ELO).arg(rank);

    QLabel *userLabel = new QLabel(UsrELORank, this);
    userLabel->setAlignment(Qt::AlignCenter);
    userLabel->setTextFormat(Qt::RichText);
    userLabel->setStyleSheet("color: white;");

    imageLayout->addWidget(userLabel);

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
    mainLayout->addLayout(imageLayout);

    // ------------- BOTONES INFERIORES -------------

    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    QPushButton *leftButton = new QPushButton("Partida Amistosa", this);
    leftButton->setStyleSheet(
        "QPushButton { background-color: green; color: white; font-size: 18px; padding: 10px; border-radius: 5px; "
        "border: 2px solid #006400; }"
        "QPushButton:hover { background-color: #008000; }"
        "QPushButton:pressed { background-color: #004d00; }"
        );
    leftButton->setFixedSize(200, 50);

    QPushButton *rightButton = new QPushButton("Jugar en Equipo", this);
    rightButton->setStyleSheet(
        "QPushButton { background-color: green; color: white; font-size: 18px; padding: 10px; border-radius: 5px; "
        "border: 2px solid #006400; }"
        "QPushButton:hover { background-color: #008000; }"
        "QPushButton:pressed { background-color: #004d00; }"
        );
    rightButton->setFixedSize(200, 50);

    buttonsLayout->addWidget(leftButton, 0, Qt::AlignLeft);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(rightButton, 0, Qt::AlignRight);

    // ------------- DETECTAR BOTONES INFERIORES -------------

    connect(leftButton, &QPushButton::clicked, this, []() {
        qDebug() << "Botón Partida Amistosa presionado";
    });

    connect(rightButton, &QPushButton::clicked, this, []() {
        qDebug() << "Botón Jugar en Equipo presionado";
    });

    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}


/**
 * @brief Convierte una imagen en un círculo.
 * @param imagePath Ruta de la imagen a cargar.
 * @param size Tamaño deseado del círculo (ancho y alto serán iguales).
 * @return QPixmap con la imagen recortada en forma circular.
 */
QPixmap UserProfileWindow::createCircularImage(const QString &imagePath, int size) {
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

