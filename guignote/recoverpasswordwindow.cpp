/**
 * @file recoverpasswordwindow.cpp
 * @brief Implementación de la clase RecoverPasswordWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Esta clase representa el diálogo para recuperar la contraseña del usuario. Permite al usuario
 * ingresar su correo electrónico para recibir instrucciones de recuperación. Además, se encarga de
 * mostrar un overlay semitransparente en el widget padre para enfocar la atención en el diálogo,
 * y reposiciona el mismo si el widget padre se mueve o redimensiona.
 */

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

//---------------------------------------------------------------------------
/**
 * @brief Constructor de RecoverPasswordWindow.
 *
 * Configura el diálogo para que sea modal, sin bordes y con un estilo personalizado. Se
 * carga una fuente personalizada para el título y se establece el layout principal que contiene
 * el título, instrucciones, campo de correo, botón para enviar y enlace para volver al inicio de sesión.
 *
 * @param parent Puntero al widget padre, por defecto nullptr.
 */
RecoverPasswordWindow::RecoverPasswordWindow(QWidget *parent)
    : QDialog(parent), backgroundOverlay(nullptr)
{
    // Configuración del diálogo: sin bordes, modal, tamaño fijo y estilo personalizado.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
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

    // Crear el layout principal vertical y configurar márgenes, espacios y alineación.
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

    // Crear y configurar el label de información con instrucciones.
    QLabel *infoLabel = new QLabel("Introduce tu correo electrónico para recibir instrucciones para recuperar tu contraseña.", this);
    infoLabel->setStyleSheet("color: #ffffff; font-size: 14px;");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);

    // Definir el estilo común para los campos de entrada.
    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 18px;"
        "   padding: 8px 10px;"
        "}";

    // Crear el campo de correo electrónico con su icono.
    QLineEdit *emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("Correo Electrónico");
    emailEdit->setStyleSheet(lineEditStyle);
    emailEdit->setFixedWidth(250);
    emailEdit->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    mainLayout->addWidget(emailEdit, 0, Qt::AlignCenter);

    // Crear el botón para enviar las instrucciones de recuperación con estilo personalizado.
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

    // Crear un layout horizontal para el enlace que permite volver al inicio de sesión.
    QHBoxLayout *extraLayout = new QHBoxLayout();
    QPushButton *loginButtonLink = new QPushButton("Volver al inicio de sesión", this);
    loginButtonLink->setStyleSheet("QPushButton { color: #ffffff; text-decoration: underline; font-size: 14px; background: transparent; border: none; }");
    extraLayout->addWidget(loginButtonLink);
    mainLayout->addLayout(extraLayout);

    // Conexión: al pulsar el enlace, se cierra este diálogo.
    connect(loginButtonLink, &QPushButton::clicked, [=]() {
        this->close();
    });
    // Otra conexión para asegurarse de cerrar el diálogo.
    connect(loginButtonLink, &QPushButton::clicked, this, &QDialog::close);
}

//---------------------------------------------------------------------------
/**
 * @brief Destructor de RecoverPasswordWindow.
 *
 * Libera los recursos utilizados por el diálogo. La eliminación de widgets hijos es gestionada
 * automáticamente por Qt.
 */
RecoverPasswordWindow::~RecoverPasswordWindow() {}

//---------------------------------------------------------------------------
/**
 * @brief Reimplementación del evento showEvent.
 *
 * Al mostrar el diálogo se crea un overlay semitransparente sobre el widget padre para enfocar la
 * atención en el diálogo. También se instala un filtro de eventos en el widget padre y se centra
 * el diálogo respecto a éste.
 *
 * @param event Evento de tipo QShowEvent.
 */
void RecoverPasswordWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (parentWidget()) {
        // Crear overlay semitransparente sobre el widget padre.
        backgroundOverlay = new QWidget(parentWidget());
        backgroundOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 128);");
        backgroundOverlay->setGeometry(parentWidget()->rect());
        backgroundOverlay->show();
        backgroundOverlay->raise();

        // Instalar filtro de eventos en el widget padre para detectar cambios de tamaño o posición.
        parentWidget()->installEventFilter(this);

        // Centrar el diálogo respecto al widget padre.
        this->move(parentWidget()->geometry().center() - this->rect().center());
    }
}

//---------------------------------------------------------------------------
/**
 * @brief Reimplementación del evento closeEvent.
 *
 * Al cerrar el diálogo se remueve el filtro de eventos instalado en el widget padre y se elimina
 * el overlay.
 *
 * @param event Evento de tipo QCloseEvent.
 */
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

//---------------------------------------------------------------------------
/**
 * @brief Reimplementación del filtro de eventos.
 *
 * Este método permite detectar eventos de movimiento o redimensionamiento en el widget padre,
 * de modo que el diálogo se recalcule y se mantenga centrado, y el overlay se ajuste a su tamaño.
 *
 * @param watched Objeto que recibe el evento.
 * @param event Evento que ocurre (movimiento o redimensionamiento).
 * @return true si el evento es procesado, false en caso contrario.
 */
bool RecoverPasswordWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        if (parentWidget()) {
            // Recentrar el diálogo respecto al widget padre.
            this->move(parentWidget()->geometry().center() - this->rect().center());
            // Actualizar el tamaño del overlay para que cubra el widget padre.
            if (backgroundOverlay) {
                backgroundOverlay->setGeometry(parentWidget()->rect());
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

/**
 * @brief Sobrescribe el método reject() para limpiar recursos adicionales.
 *
 * Este método se encarga de realizar tareas de limpieza específicas antes de cerrar
 * el diálogo de inicio de sesión. En particular, elimina el filtro de eventos instalado
 * en el widget padre y programa la eliminación del overlay semitransparente (fondo oscuro)
 * que se aplicó al widget padre para enfocar la atención en el diálogo.
 *
 * Estas acciones aseguran que, al cerrarse el diálogo (por ejemplo, al presionar Escape),
 * no queden recursos residuales que bloqueen la interacción con la ventana principal.
 */
void RecoverPasswordWindow::reject() {
    // Verifica que exista un widget padre y remueve el filtro de eventos instalado.
    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    // Si el overlay de fondo está activo, lo elimina de forma segura y lo desasigna.
    if (backgroundOverlay) {
        backgroundOverlay->deleteLater();
        backgroundOverlay = nullptr;
    }
    // Llama al método reject() de la clase base para ejecutar el cierre del diálogo.
    QDialog::reject();
}
