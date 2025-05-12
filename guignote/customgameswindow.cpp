/**
 * @file customgameswindow.cpp
 * @brief Implementación de la ventana para explorar y unirse a partidas personalizadas.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 */

#include "customgameswindow.h"
#include "crearcustomgame.h"
#include "estadopartida.h"
#include "menuwindow.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCheckBox>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QString>
#include <QNetworkAccessManager>
#include <QApplication>

/**
 * @brief Constructor de CustomGamesWindow.
 * @param userKey Clave del usuario para autenticación.
 * @param usr Nombre del usuario.
 * @param fondo Identificador visual del fondo.
 * @param parent Widget padre.
 */
CustomGamesWindow::CustomGamesWindow(const QString &userKey, QString usr, int fondo, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(900, 650);

    networkManager = new QNetworkAccessManager(this);
    this->userKey = userKey;
    this->usr = usr;
    this->fondo = fondo;
    token = loadAuthToken(userKey);
    setupUI();

    fetchAllGames();
}

/**
 * @brief Carga el token de autenticación desde archivo de configuración local.
 * @param userKey Clave de usuario usada para localizar el archivo.
 * @return Token JWT como QString, o cadena vacía si no se encuentra.
 */
QString CustomGamesWindow::loadAuthToken(const QString &userKey) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
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

/**
 * @brief Configura todos los elementos gráficos de la ventana.
 */
void CustomGamesWindow::setupUI() {
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // ——— Header (título + botón de cerrar) ———
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Partidas Personalizadas", this);
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
    connect(closeButton, &QPushButton::clicked, this, &CustomGamesWindow::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // ——— Checkbox “Solo amigos” ———
    static const QString checkboxStyle = R"(
        QCheckBox { color: #ffffff; font-size: 14px; }
        QCheckBox::indicator { width: 16px; height: 16px; }
        QCheckBox::indicator:unchecked {
            background-color: #c2c2c3;
            border: 1px solid #545454;
        }
        QCheckBox::indicator:checked {
            background-color: #c2c2c3;
            border: 1px solid #545454;
            image: url(:/icons/cross.png);
        }
    )";

    soloAmigosCheck = new QCheckBox("Solo Amigos", this);
    soloAmigosCheck->setStyleSheet(checkboxStyle);
    connect(soloAmigosCheck, &QCheckBox::checkStateChanged, this, [this](){
        soloAmigos = !soloAmigos;
        if (!soloAmigos) {
            fetchAllGames();
        } else {
           fetchFriendGames();
        }
    });

    // ——— Botón “Crear Partida” ———
    QPushButton *createGameButton = new QPushButton("Crear Partida", this);
    createGameButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #45A049; }"
        );
    connect(createGameButton, &QPushButton::clicked, this, [this](){
        CrearCustomGame *createWin = new CrearCustomGame(userKey, usr, fondo, this);
        createWin->setModal(true);
        createWin->exec();
    });

    // ——— Layout de controles (botón + checkbox) ———
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);
    controlsLayout->addWidget(createGameButton);
    controlsLayout->addWidget(soloAmigosCheck);

    // Centrar horizontalmente en la parte superior
    mainLayout->addLayout(controlsLayout);
    mainLayout->setAlignment(controlsLayout, Qt::AlignHCenter);

    // ...añade aquí el resto de widgets si hace falta...
}

/**
 * @brief Obtiene la lista de partidas personalizadas disponibles desde el servidor.
 *
 * Esta función realiza una petición HTTP al backend para obtener las salas disponibles.
 * Las salas se muestran en la interfaz con su información básica y opción de unirse.
 */
void CustomGamesWindow::fetchAllGames() {
    qDebug() << "Fetching all available games";

    QNetworkRequest request(QUrl("http://188.165.76.134:8000/salas/disponibles/?solo_personalizadas=true"));
    request.setRawHeader("Auth", token.toUtf8());  // Añadir el token de autenticación

    // Realizar la solicitud GET
    QNetworkReply *reply = networkManager->get(request);

    // Conectar la señal finished() con un slot para manejar la respuesta
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Manejar la respuesta de la solicitud
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error fetching available games:" << reply->errorString();
            qDebug() << "Response code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            reply->deleteLater();
            return;
        }

        // Leer los datos JSON de la respuesta
        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "Error parsing JSON response:" << parseError.errorString();
            reply->deleteLater();
            return;
        }

        QJsonObject jsonObject = doc.object();

        // Comprobar si el campo 'salas' existe y es un array
        if (!jsonObject.contains("salas") || !jsonObject["salas"].isArray()) {
            qDebug() << "No 'salas' field in the response or it's not an array.";
            reply->deleteLater();
            return;
        }

        // Obtener el array de salas
        QJsonArray salasArray = jsonObject["salas"].toArray();

        // Limpiar la lista antes de agregar los nuevos elementos
        for (int i = 0; i < mainLayout->count(); ++i) {
            QLayoutItem *item = mainLayout->itemAt(i);
            if (item != nullptr) {
                QWidget *widget = item->widget();
                if (widget != nullptr) {
                    widget->deleteLater();  // Eliminar los widgets del layout
                }
            }
        }

        // Iterar sobre el array de salas y agregar un item por cada sala
        for (const QJsonValue &value : salasArray) {
            QJsonObject salaObject = value.toObject();
            QString nombreSala = salaObject["nombre"].toString();
            int idSala = salaObject["id"].toInt();
            capacidad = salaObject["capacidad"].toInt();
            int numJugadores = salaObject["num_jugadores"].toInt();

            // ——— Crear un contenedor con fondo más claro ———
            QWidget *container = new QWidget(this);
            container->setStyleSheet(
                "background-color: #232326;"   // ligeramente más claro que #171718
                "border-radius: 10px;"
                "padding: 10px;"
                );

            // Le damos un layout horizontal
            QHBoxLayout *row = new QHBoxLayout(container);
            row->setContentsMargins(0, 0, 0, 0);
            row->setSpacing(15);

            // ——— Label con nombre y ocupación ———
            QLabel *label = new QLabel(
                QString("%1  —  %2/%3")
                    .arg(nombreSala)
                    .arg(numJugadores)
                    .arg(capacidad),
                container
                );
            label->setStyleSheet("color: white; font-size: 18px;");

            // ——— Detalles de personalización ———
            QString personalizacionStr;
            if (salaObject.contains("personalizacion") && salaObject["personalizacion"].isObject()) {
                QJsonObject personalizacion = salaObject["personalizacion"].toObject();
                personalizacionStr = "Tiempo Turno: " + QString::number(personalizacion["tiempo_turno"].toInt()) + "s, "
                                     + "Reglas Arrastre: " + (personalizacion["reglas_arrastre"].toBool() ? "Sí" : "No") + ", "
                                     + "Revueltas: " + (personalizacion["permitir_revueltas"].toBool() ? "Sí" : "No") + ", "
                                     + "Solo Amigos: " + (personalizacion["solo_amigos"].toBool() ? "Sí" : "No");
            }

            QLabel *personalizacionLabel = new QLabel(personalizacionStr, container);
            personalizacionLabel->setStyleSheet("color: white; font-size: 14px;");

            // ——— Botón verde Rejoin ———
            QPushButton *rejoinButton = new QPushButton("Unirse", container);
            rejoinButton->setStyleSheet(
                "QPushButton {"
                "  background-color: #28a745;"
                "  color: white;"
                "  border-radius: 5px;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #218838;"
                "}"
                );
            int cap = capacidad;
            connect(rejoinButton, &QPushButton::clicked, this, [this, idSala, cap]() {
                qDebug() << "Joining Room ID:" << idSala;
                joinGame(QString::number(idSala), cap);
            });

            // ——— Montamos la fila dentro del contenedor ———
            row->addWidget(label);
            row->addStretch();
            row->addWidget(personalizacionLabel);  // Añadir la etiqueta de personalización
            row->addWidget(rejoinButton);  // Añadir el botón "Rejoin"
            container->setLayout(row);

            // ——— Añadimos un pequeño margen vertical entre contenedores ———
            mainLayout->addWidget(container);
            mainLayout->setSpacing(10);
        }

        reply->deleteLater();  // Eliminar el reply después de procesar la respuesta
    });
}


/**
 * @brief Obtiene la lista de partidas disponibles solo entre amigos.
 *
 * Realiza una solicitud HTTP al backend para obtener únicamente las salas
 * creadas por amigos del usuario. Los resultados se renderizan en la interfaz.
 */
void CustomGamesWindow::fetchFriendGames() {
    qDebug() << "Fetching friend-only games";

    // Crear la solicitud GET para obtener las salas de amigos
    QNetworkRequest request(QUrl("http://188.165.76.134:8000/salas/disponibles/amigos"));
    request.setRawHeader("Auth", token.toUtf8());  // Añadir el token de autenticación

    // Realizar la solicitud GET
    QNetworkReply *reply = networkManager->get(request);

    // Conectar la señal finished() con un slot para manejar la respuesta
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Manejar la respuesta de la solicitud
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error fetching friend games:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        // Leer los datos JSON de la respuesta
        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "Error parsing JSON response:" << parseError.errorString();
            reply->deleteLater();
            return;
        }

        QJsonObject jsonObject = doc.object();

        // Comprobar si el campo 'salas' existe y es un array
        if (!jsonObject.contains("salas") || !jsonObject["salas"].isArray()) {
            qDebug() << "No 'salas' field in the response or it's not an array.";
            reply->deleteLater();
            return;
        }

        // Obtener el array de salas
        QJsonArray salasArray = jsonObject["salas"].toArray();

        // Limpiar la lista antes de agregar los nuevos elementos
        for (int i = 0; i < mainLayout->count(); ++i) {
            QLayoutItem *item = mainLayout->itemAt(i);
            if (item != nullptr) {
                QWidget *widget = item->widget();
                if (widget != nullptr) {
                    widget->deleteLater();  // Eliminar los widgets del layout
                }
            }
        }

        // Iterar sobre el array de salas y agregar un item por cada sala
        for (const QJsonValue &value : salasArray) {
            QJsonObject salaObject = value.toObject();
            QString nombreSala = salaObject["nombre"].toString();
            int idSala = salaObject["id"].toInt();
            capacidad = salaObject["capacidad"].toInt();
            int numJugadores = salaObject["num_jugadores"].toInt();

            // ——— Crear un contenedor con fondo más claro ———
            QWidget *container = new QWidget(this);
            container->setStyleSheet(
                "background-color: #232326;"   // ligeramente más claro que #171718
                "border-radius: 10px;"
                "padding: 10px;"
                );

            // Le damos un layout horizontal
            QHBoxLayout *row = new QHBoxLayout(container);
            row->setContentsMargins(0, 0, 0, 0);
            row->setSpacing(15);

            // ——— Label con nombre y ocupación ———
            QLabel *label = new QLabel(
                QString("%1  —  %2/%3")
                    .arg(nombreSala)
                    .arg(numJugadores)
                    .arg(capacidad),
                container
                );
            label->setStyleSheet("color: white; font-size: 18px;");

            // ——— Detalles de personalización ———
            QJsonObject personalizacion = salaObject["personalizacion"].toObject();
            QString personalizacionStr = "Tiempo Turno: " + QString::number(personalizacion["tiempo_turno"].toInt()) + "s, "
                                         + "Reglas Arrastre: " + (personalizacion["reglas_arrastre"].toBool() ? "Sí" : "No") + ", "
                                         + "Revueltas: " + (personalizacion["permitir_revueltas"].toBool() ? "Sí" : "No") + ", "
                                         + "Solo Amigos: " + (personalizacion["solo_amigos"].toBool() ? "Sí" : "No");

            QLabel *personalizacionLabel = new QLabel(personalizacionStr, container);
            personalizacionLabel->setStyleSheet("color: white; font-size: 14px;");

            // ——— Botón verde Rejoin ———
            QPushButton *joinButton = new QPushButton("Unirse", container);
            joinButton->setStyleSheet(
                "QPushButton {"
                "  background-color: #28a745;"
                "  color: white;"
                "  border-radius: 5px;"
                "  padding: 6px 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #218838;"
                "}"
                );
            int cap = capacidad;
            connect(joinButton, &QPushButton::clicked, this, [this, idSala, cap]() {
                qDebug() << "Joining Room ID:" << idSala;
                joinGame(QString::number(idSala), cap);
            });

            // ——— Montamos la fila dentro del contenedor ———
            row->addWidget(label);
            row->addStretch();
            row->addWidget(personalizacionLabel);  // Añadir la etiqueta de personalización
            row->addWidget(joinButton);
            container->setLayout(row);

            // ——— Añadimos un pequeño margen vertical entre contenedores ———
            mainLayout->addWidget(container);
            mainLayout->setSpacing(10);
        }

        reply->deleteLater();  // Eliminar el reply después de procesar la respuesta
    });
}

/**
 * @brief Une al usuario a una partida personalizada existente.
 *
 * Conecta mediante WebSocket a una sala concreta utilizando su ID y capacidad.
 * Muestra la ventana de juego y cierra otras ventanas de nivel superior.
 *
 * @param idPart ID de la partida a la que se desea unir.
 * @param cap Capacidad total de la partida.
 */
void CustomGamesWindow::joinGame(QString idPart, int cap){

    QString url = QString("ws://188.165.76.134:8000/ws/partida/?token=%1&id_partida=%2&capacidad=%3")
                      .arg(token)
                      .arg(idPart)
                      .arg(cap);
    qDebug() << "Conectando a:" << url;

    // Creamos la nueva ventana (EstadoPartida o GameWindow)
    EstadoPartida *gameWindow = new EstadoPartida(usr, userKey, url, /** tapete */ 1, /** skin */ 1, [this]() {
        auto* menu = new MenuWindow(userKey);
        menu->showFullScreen();
    });

    gameWindow->setAttribute(Qt::WA_DeleteOnClose);
    gameWindow->setWindowFlag(Qt::Window); // Asegura que sea una ventana independiente

    // Le damos la misma geometría que el menú actual
    gameWindow->showFullScreen();

    QTimer::singleShot(125, gameWindow, [gameWindow]() {
        gameWindow->init();
    });

    /* ⚠️ Parar música antes de cerrar
    if (backgroundPlayer) {
        backgroundPlayer->stop();
        backgroundPlayer->deleteLater();
        backgroundPlayer = nullptr;
    }
    */

    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (w != gameWindow) {
            w->close();
        }
    }
}

