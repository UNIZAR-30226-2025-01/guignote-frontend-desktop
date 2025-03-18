#include "friendsmessagewindow.h"
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QStandardPaths>


FriendsMessageWindow::FriendsMessageWindow(QWidget *parent, QString ID, QString Usuario)
    : QWidget(parent)
{
    friendID = ID;
    usr = Usuario;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    setupUI();
}

void FriendsMessageWindow::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // Encabezado: Título y botón de cierre.
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel(usr, this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");

    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(22, 22));
    closeButton->setFixedSize(35, 35);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 17px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // Entrada de mensajes
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("...");
    messageInput->setStyleSheet(
        "background-color: #292A2D; color: white; font-size: 16px; padding: 10px; "
        "border-radius: 10px; border: 1px solid #444;"
        );
    messageInput->setFixedHeight(40);

    connect(messageInput, &QLineEdit::returnPressed, this, &FriendsMessageWindow::sendMessage);

    // Se añade un `QSpacerItem` para empujar el campo de texto hacia abajo
    mainLayout->addStretch();
    mainLayout->addWidget(messageInput);
}

void FriendsMessageWindow::sendMessage()
{
    QString message = messageInput->text(); // Obtiene el texto ingresado
    if (!message.isEmpty()) {

        qDebug() << "Sent";

        messageInput->clear(); // Borra el campo de texto después de enviar el mensaje
    }
}

void FriendsMessageWindow::loadMessages()
{

}

// Función para extraer el token de autenticación desde el archivo .conf
QString FriendsMessageWindow::loadAuthToken() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + "/Grace Hopper/Sota, Caballo y Rey.conf";
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo cargar el archivo de configuración.";
        return "";
    }
    QString token;
    while (!configFile.atEnd()) {
        QString line = configFile.readLine().trimmed();
        if (line.startsWith("token=")) {
            token = line.mid(QString("token=").length()).trimmed();
            break;
        }
    }
    configFile.close();
    if (token.isEmpty()) {
        qDebug() << "No se encontró el token en el archivo de configuración.";
    }
    return token;
}
