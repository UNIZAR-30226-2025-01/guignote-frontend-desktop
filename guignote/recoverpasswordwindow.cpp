#include "recoverpasswordwindow.h"
#include "loginwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFontDatabase>
#include <QIcon>
#include <QAction>
#include <QDebug>
#include <QShowEvent>
#include <QCloseEvent>
#include <QEvent>

// Constructor de RecoverPasswordWindow. Se configura el diálogo para que sea modal y se define su estilo.
RecoverPasswordWindow::RecoverPasswordWindow(QWidget *parent)
    : QDialog(parent), backgroundOverlay(nullptr) // Se inicializa backgroundOverlay en nullptr
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    // Configurar el diálogo como modal y establecer dimensiones fijas y estilo personalizado.
    setModal(true);
    setFixedSize(480, 500);
    setStyleSheet("background-color: #171718; border-radius: 5px; padding: 20px;");

    // Cargar la fuente personalizada para el título.
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QFont titleFont;
    if (fontId != -1) {
        titleFont = QFont(QFontDatabase::applicationFontFamilies(fontId).at(0), 32);
    } else {
        qWarning() << "No se pudo cargar la fuente personalizada.";
        titleFont = QFont("Arial", 32, QFont::Bold);
    }

    // Crear el layout principal (vertical) y configurar márgenes, espacios y alineación.
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Crear y configurar el label del título.
    QLabel *titleLabel = new QLabel("Recuperar Contraseña", this);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Crear y configurar el label de información con las instrucciones para el usuario.
    QLabel *infoLabel = new QLabel("Introduce tu correo electrónico para recibir instrucciones para recuperar tu contraseña.", this);
    infoLabel->setStyleSheet("color: #ffffff; font-size: 14px;");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);

    // Definir el estilo común para los QLineEdit.
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Crear el campo para introducir el correo electrónico, con su icono y estilo.
    QLineEdit *emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Correo Electrónico");
    emailEdit->setStyleSheet(lineEditStyle);
    emailEdit->setFixedWidth(250);
    emailEdit->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(emailEdit, 0, Qt::AlignCenter);

    // Crear el botón para enviar las instrucciones de recuperación, con su estilo personalizado.
    QPushButton *recoverButton = new QPushButton("Enviar", this);
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   padding: 12px 25px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #9b9b9b;"
        "}";
    recoverButton->setStyleSheet(buttonStyle);
    recoverButton->setFixedSize(200, 50);
    mainLayout->addWidget(recoverButton, 0, Qt::AlignCenter);

    // Crear un layout horizontal para el enlace que permite volver a la ventana de inicio de sesión.
    QHBoxLayout *extraLayout = new QHBoxLayout();
    QPushButton *loginButtonLink = new QPushButton("Volver al inicio de sesión", this);
    loginButtonLink->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout->addWidget(loginButtonLink);
    mainLayout->addLayout(extraLayout);

    // Conexión: Al pulsar el enlace se crea y muestra la ventana de inicio de sesión y se cierra este diálogo.
    connect(loginButtonLink, &QPushButton::clicked, [=]() {
        this->close();
    });
    connect(loginButtonLink, &QPushButton::clicked, this, &QDialog::close);
}

RecoverPasswordWindow::~RecoverPasswordWindow() {}

// Reimplementación del método showEvent para crear un overlay y centrar el diálogo respecto al padre.
void RecoverPasswordWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (parentWidget()) {
        // Crear un overlay que oscurezca el fondo del widget padre.
        backgroundOverlay = new QWidget(parentWidget());
        backgroundOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 128);");
        backgroundOverlay->setGeometry(parentWidget()->rect());
        backgroundOverlay->show();
        backgroundOverlay->raise();

        // Instalar un filtro de eventos en el widget padre para detectar eventos de movimiento y redimensionamiento.
        parentWidget()->installEventFilter(this);

        // Centrar este diálogo en el centro del widget padre.
        this->move(parentWidget()->geometry().center() - this->rect().center());
    }
}

// Reimplementación del método closeEvent para remover el filtro de eventos y eliminar el overlay.
void RecoverPasswordWindow::closeEvent(QCloseEvent *event)
{
    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    if (backgroundOverlay) {
        backgroundOverlay->deleteLater();
        backgroundOverlay = nullptr;
    }
    QDialog::closeEvent(event);
}

// Reimplementación del eventFilter para reposicionar el diálogo y actualizar el overlay al mover o redimensionar el widget padre.
bool RecoverPasswordWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        if (parentWidget()) {
            // Recentrar el diálogo en el widget padre.
            this->move(parentWidget()->geometry().center() - this->rect().center());
            // Actualizar el tamaño del overlay para que cubra el área completa del widget padre.
            if (backgroundOverlay) {
                backgroundOverlay->setGeometry(parentWidget()->rect());
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}
