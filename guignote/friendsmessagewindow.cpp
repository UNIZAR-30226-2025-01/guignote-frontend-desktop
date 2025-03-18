#include "friendsmessagewindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QTimer>




FriendsMessageWindow::FriendsMessageWindow(QWidget *parent, QString ID, QString Usuario)
    : QWidget(parent)
{
    friendID = ID;
    usr = Usuario;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(600, 680);

    networkManager = new QNetworkAccessManager(this);

    setupUI();
    loadMessages();

    messageTimer = new QTimer(this);
    connect(messageTimer, &QTimer::timeout, this, &FriendsMessageWindow::loadMessages);

    messageTimer->start(1000);
}

void FriendsMessageWindow::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // Encabezado
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

    // **Lista donde se mostrarán los mensajes**
    messagesListWidget = new QListWidget(this);
    messagesListWidget->setStyleSheet(
        "background-color: #292A2D; color: white; font-size: 16px; padding: 10px; "
        "border-radius: 10px; border: 1px solid #444;"
        );
    mainLayout->addWidget(messagesListWidget);

    // Entrada de mensajes
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Escribe un mensaje...");
    messageInput->setStyleSheet(
        "background-color: #292A2D; color: white; font-size: 16px; padding: 10px; "
        "border-radius: 10px; border: 1px solid #444;"
        );
    messageInput->setFixedHeight(40);
    connect(messageInput, &QLineEdit::returnPressed, this, &FriendsMessageWindow::sendMessage);

    mainLayout->addWidget(messageInput);
}


void FriendsMessageWindow::sendMessage()
{
    // Obtener el texto ingresado en el campo de entrada
    QString message = messageInput->text().trimmed();
    messageInput->clear();

    // Verificar que el mensaje no esté vacío
    if (message.isEmpty()) {
        qDebug() << "Intento de enviar mensaje vacío.";
        return;
    }

    // Cargar el token de autenticación desde el archivo de configuración
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "No se pudo obtener el token de autenticación.";
        return;
    }

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/mensajes/enviar/"));
    request.setRawHeader("Auth", token.toUtf8());

    QJsonObject json;
    json["receptor_id"] = friendID;
    json["contenido"] = message;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    QNetworkReply *reply = networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QByteArray responseData = reply->readAll();
        qDebug() << "Respuesta del servidor:" << responseData;
    });

}


void FriendsMessageWindow::loadMessages()
{
    QString token = loadAuthToken();
    if (token.isEmpty()) {
        qDebug() << "No se pudo obtener el token de autenticación.";
        return;
    }

    QUrl url(QString("http://188.165.76.134:8000/mensajes/obtener/?receptor_id=%1").arg(friendID));
    QNetworkRequest request(url);
    request.setRawHeader("Auth", token.toUtf8());

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QByteArray responseData = reply->readAll();
        qDebug() << "Respuesta del servidor (mensajes):" << responseData;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            qDebug() << "JSON recibido del servidor:" << doc.toJson(QJsonDocument::Indented); // 🔹 Debug JSON completo

            if (!doc.isObject()) {
                qDebug() << "Error: Respuesta del servidor no es un JSON válido.";
                return;
            }

            QJsonObject jsonResponse = doc.object();
            if (!jsonResponse.contains("mensajes")) {
                qDebug() << "No se encontraron mensajes en la respuesta.";
                return;
            }

            QJsonArray messagesArray = jsonResponse["mensajes"].toArray();
            messagesListWidget->clear();

            for (int i = messagesArray.size() - 1; i >= 0; --i) {
                if (!messagesArray[i].isObject()) {
                    qDebug() << "Error: messagesArray[" << i << "] no es un objeto JSON válido.";
                    continue;
                }

                QJsonObject messageObject = messagesArray[i].toObject();

                if (!messageObject.contains("emisor")) {
                    qDebug() << "Error: La clave 'emisor' no existe en el mensaje.";
                    qDebug() << "Contenido del objeto JSON:" << messageObject;
                    continue;
                }

                // 🔹 Convertir correctamente "emisor" a QString
                QString senderId = QString::number(messageObject["emisor"].toInt());
                QString content = messageObject["contenido"].toString();

                QLabel *messageLabel = new QLabel(content);
                messageLabel->setWordWrap(true);
                messageLabel->setContentsMargins(10, 5, 10, 5);

                if (senderId != friendID) {
                    messageLabel->setStyleSheet("background-color: #2196F3; color: white; padding: 8px; border-radius: 8px;");
                    messageLabel->setAlignment(Qt::AlignRight);
                } else {
                    messageLabel->setStyleSheet("background-color: white; color: black; padding: 8px; border-radius: 8px;");
                    messageLabel->setAlignment(Qt::AlignLeft);
                }

                QListWidgetItem *item = new QListWidgetItem();
                item->setSizeHint(messageLabel->sizeHint());
                messagesListWidget->addItem(item);
                messagesListWidget->setItemWidget(item, messageLabel);
            }
        } else {
            qDebug() << "Error al cargar mensajes:" << reply->errorString();
        }

        reply->deleteLater();
    });
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
