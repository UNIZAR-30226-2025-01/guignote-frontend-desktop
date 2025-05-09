#include "customgameswindow.h"
#include "crearcustomgame.h"
#include "gamewindow.h"

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
            int capacidad = salaObject["capacidad"].toInt();
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
            QPushButton *rejoinButton = new QPushButton("Rejoin", container);
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
            connect(rejoinButton, &QPushButton::clicked, this, [this, idSala]() {
                qDebug() << "Joining Room ID:" << idSala;
                joinGame(QString::number(idSala));
            });

            // ——— Montamos la fila dentro del contenedor ———
            row->addWidget(label);
            row->addStretch();
            row->addWidget(rejoinButton);  // Añadir el botón "Rejoin"
            row->addWidget(personalizacionLabel);  // Añadir la etiqueta de personalización
            container->setLayout(row);

            // ——— Añadimos un pequeño margen vertical entre contenedores ———
            mainLayout->addWidget(container);
            mainLayout->setSpacing(10);
        }

        reply->deleteLater();  // Eliminar el reply después de procesar la respuesta
    });
}



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
            int capacidad = salaObject["capacidad"].toInt();
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
            connect(joinButton, &QPushButton::clicked, this, [this, idSala]() {
                qDebug() << "Joining Room ID:" << idSala;
                joinGame(QString::number(idSala));
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

void CustomGamesWindow::manejarMensaje(const QString &userKey, const QString &mensaje) {
    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "❌ Mensaje no es JSON válido.";
        return;
    }

    QWidget *w = parentWidget();
    MenuWindow *menuWin = qobject_cast<MenuWindow*>(w);

    QJsonObject root = doc.object();
    QString tipo = root.value("type").toString();
    QJsonObject data = root.value("data").toObject();

    if (tipo == "player_joined") {
        QString nombre = data["usuario"].toObject()["nombre"].toString();
        int id = data["usuario"].toObject()["id"].toInt();
        int chatId = data["chat_id"].toInt();

        qDebug() << "ChatID: " << chatId;

        qDebug() << "👤 Se ha unido un jugador:" << nombre << "(ID:" << id << ")";

        if(nombre == usr){
            this->id = id;
        }
    }

    else if (tipo == "start_game") {
        int mazoRestante = data["mazo_restante"].toInt();
        bool faseArrastre = data["fase_arrastre"].toBool();
        int chatId = data["chat_id"].toInt();

        QJsonObject cartaTriunfo = data["carta_triunfo"].toObject();
        QString paloTriunfo = cartaTriunfo["palo"].toString();
        int valorTriunfo = cartaTriunfo["valor"].toInt();

        QJsonArray cartasJugador = data["mis_cartas"].toArray();
        QJsonArray jugadores = data["jugadores"].toArray();

        qDebug() << "🎮 Inicio de partida";
        qDebug() << "🃏 Triunfo:" << paloTriunfo << valorTriunfo;
        qDebug() << "📦 Cartas en mazo:" << mazoRestante;
        qDebug() << "💬 Chat ID:" << chatId;

        // — Nuevo: listamos id y número de cartas de cada jugador —
        for (const QJsonValue &v : jugadores) {
            if (!v.isObject()) continue;
            QJsonObject jo = v.toObject();
            int pid        = jo.value("id").toInt();
            int numCartas  = jo.value("num_cartas").toInt();
            qDebug() << "👤 Jugador" << pid << "→" << numCartas << "cartas";
        }

        int numJugadores = jugadores.size();
        qDebug() << "hay " << numJugadores << " jugadores";
        int type = -1;
        switch (numJugadores) {
        case 2:
            type = 1;
            break;
        case 4:
            type = 2;
            break;
        default:
            break;
        }

        // — Construimos el GameWindow y lo colocamos donde estaba el menú —
        GameWindow *gameWindow = new GameWindow(
            userKey, type, 1, data, id, webSocket, usr, menuWin
            );
        gameWindow->setGeometry(w->geometry());
        gameWindow->show();

        for (QWidget *win : QApplication::topLevelWidgets()) {
            if (win != gameWindow) {
                win->close();
            }
        }
    }

}

void CustomGamesWindow::joinGame(QString idPart){
    qDebug() << "creamos socket";
    webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    // conexión original
    connect(webSocket, &QWebSocket::connected, [this]() {
        qDebug() << "WebSocket conectado correctamente.";
    });

    connect(webSocket, &QWebSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
        qDebug() << "Error en WebSocket:" << error;
    });
    connect(webSocket, &QWebSocket::textMessageReceived, this, [=](const QString &mensaje) {
        this->manejarMensaje(userKey, mensaje);
    });

    QString url = QString("ws://188.165.76.134:8000/ws/partida/?token=%1&id_partida=%2")
                      .arg(token)
                      .arg(idPart);
    qDebug() << "Conectando a:" << url;
    webSocket->open(QUrl(url));

    // ————————————— Mostrar cuadro modal mientras busca partida —————————————
    if (searchingDialog) {
        searchingDialog->deleteLater();
    }
    searchingDialog = new QDialog(this);
    searchingDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    searchingDialog->setModal(true);
    searchingDialog->setStyleSheet(
        "QDialog { background-color: #171718; border-radius: 5px; padding: 20px; }"
        );

    // sombra
    QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(searchingDialog);
    dialogShadow->setBlurRadius(10);
    dialogShadow->setColor(QColor(0,0,0,80));
    dialogShadow->setOffset(4,4);
    searchingDialog->setGraphicsEffect(dialogShadow);

    QVBoxLayout *searchLayout = new QVBoxLayout(searchingDialog);

    // título grande
    QLabel *searchLabel = new QLabel("Buscando oponente...", searchingDialog);
    searchLabel->setStyleSheet(
        "color: white;"
        "font-size: 28px;"
        "background: transparent;"
        );
    searchLabel->setAlignment(Qt::AlignCenter);
    searchLayout->addWidget(searchLabel);

    // candados giratorios
    QHBoxLayout *iconsLayout = new QHBoxLayout();
    iconsLayout->setSpacing(15);
    iconsLayout->setAlignment(Qt::AlignCenter);

    QVector<QLabel*> lockIcons;
    for (int i = 0; i < 3; ++i) {
        QLabel *icon = new QLabel(searchingDialog);
        icon->setStyleSheet("background: transparent;");
        icon->setPixmap(
            QPixmap(":/images/app_logo_white.png")
                .scaled(50,50, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
        iconsLayout->addWidget(icon);
        lockIcons.append(icon);
    }
    searchLayout->addLayout(iconsLayout);

    // botón cancelar
    QPushButton *cancelButton = new QPushButton("Cancelar", searchingDialog);
    cancelButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #c2c2c3;"
        "  color: #171718;"
        "  border-radius: 15px;"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "  padding: 12px 25px;"
        "}"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    cancelButton->setFixedSize(140,45);
    searchLayout->addWidget(cancelButton, 0, Qt::AlignCenter);

    searchingDialog->adjustSize();
    searchingDialog->move(this->geometry().center() - searchingDialog->rect().center());
    searchingDialog->show();

    // cerrar socket y diálogo al cancelar
    connect(cancelButton, &QPushButton::clicked, [this]() {
        if (webSocket) {
            webSocket->close();
            webSocket->deleteLater();
            webSocket = nullptr;
        }
        if (searchingDialog) {
            searchingDialog->close();
            searchingDialog->deleteLater();
            searchingDialog = nullptr;
        }
    });

    // animación de giro (más lenta)
    QTimer *rotateTimer = new QTimer(searchingDialog);
    connect(rotateTimer, &QTimer::timeout, [lockIcons]() {
        static int angle = 0;
        angle = (angle + 10) % 360;
        for (QLabel *icon : lockIcons) {
            QPixmap orig(":/images/app_logo_white.png");
            QTransform tr; tr.rotate(angle);
            icon->setPixmap(
                orig.transformed(tr, Qt::SmoothTransformation)
                    .scaled(50,50, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
        }
    });
    rotateTimer->start(100);
        // ——————————————————————————————————————————————————————————
}

