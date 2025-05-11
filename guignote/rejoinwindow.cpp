/**
 * @file rejoinwindow.cpp
 * @brief Implementación de la clase RejoinWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Esta clase muestra un diálogo con una lista de salas reconectables (recibidas como JSON),
 * permitiendo al usuario volver a unirse a una partida via WebSocket. Gestiona la creación de la UI,
 * el manejo de mensajes entrantes y la reconexión usando el token de autenticación.
 */


#include "rejoinwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWebSocket>
#include "menuwindow.h"

/**
 * @brief Constructor de RejoinWindow.
 * @param jsonArray Array JSON con la información de las salas.
 * @param fondo Identificador de recurso de fondo.
 * @param userKey Clave del usuario para extraer el token.
 * @param usr Nombre del usuario actual.
 * @param parent Widget padre, puede ser nullptr.
 *
 * Configura el diálogo (sin bordes y con estilo), carga el token,
 * fija tamaño y estilo, y llama a setupUI() para construir la interfaz.
 */
RejoinWindow::RejoinWindow(QJsonArray jsonArray, int fondo, QString &userKey, QString usr, QWidget *parent)
    : QDialog(parent), salas(jsonArray) {
    this->fondo = fondo;
    this->usr = usr;
    this->userKey = userKey;

    token = loadAuthToken(userKey);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(850, 680);
    setupUI();
}

/**
 * @brief Construye la interfaz de usuario.
 *
 * Crea el layout principal vertical, ajusta márgenes y espaciados,
 * añade el header y llama a populateSalas() para generar la lista.
 */
void RejoinWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(createHeaderLayout());
    populateSalas();

    setLayout(mainLayout);
}

/**
 * @brief Crea y devuelve el layout del encabezado.
 * @return Puntero al QHBoxLayout que contiene el título y el botón de cerrar.
 *
 * Configura el QLabel con el título y el QPushButton de cierre,
 * con estilo y conexiones adecuadas.
 */
QHBoxLayout* RejoinWindow::createHeaderLayout() {
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Perfil", this);
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
    return headerLayout;
}

/**
 * @brief Rellena el diálogo con las salas reconectables.
 *
 * Para cada objeto en `salas` crea un contenedor con nombre,
 * ocupación y un botón "Entrar" que llama a rejoin().
 */
void RejoinWindow::populateSalas() {
    for (const QJsonValue &val : salas) {

        // Dentro del bucle de populateSalas(), justo al inicio:
        QJsonObject obj = val.toObject();

        if (!val.isObject()) continue;

        QJsonValue idVal = obj.value("id");
        QString idStr;
        if (idVal.isString()) {
            idStr = idVal.toString();
        } else {
            // Si viene como número, lo convertimos a entero y luego a cadena
            idStr = QString::number(idVal.toInt());
        }
        QString nombre       = obj.value("nombre").toString();
        int     capacidad    = obj.value("capacidad").toInt();
        int     numJugadores = obj.value("num_jugadores").toInt();

        // ——— Creamos un contenedor con fondo más claro ———
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
                .arg(nombre)
                .arg(numJugadores)
                .arg(capacidad),
            container
            );
        label->setStyleSheet("color: white; font-size: 18px;");

        // ——— Botón verde ———
        QPushButton *btn = new QPushButton("Entrar", container);
        btn->setStyleSheet(
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
        connect(btn, &QPushButton::clicked, this, [this, idStr]() {
            qDebug() << "Sala" << idStr << "pulsada";
            rejoin(idStr);
        });

        // ——— Montamos la fila dentro del contenedor ———
        row->addWidget(label);
        row->addStretch();
        row->addWidget(btn);
        container->setLayout(row);

        // ——— Añadimos un pequeño margen vertical entre contenedores ———
        mainLayout->addWidget(container);
        mainLayout->setSpacing(10);
    }
}

/**
 * @brief Maneja mensajes entrantes por WebSocket.
 * @param userKey Clave del usuario para contexto.
 * @param mensaje Cadena JSON recibida.
 *
 * Parsear el JSON, detectar eventos ("player_joined", "start_game")
 * y actuar en consecuencia (debug, abrir GameWindow y cerrar diálogo).
 */
void RejoinWindow::manejarMensaje(const QString &userKey, const QString &mensaje) {
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
        /*GameWindow *gameWindow = new GameWindow(
            userKey, type, 1, data, id, webSocket, usr, menuWin
            );
        gameWindow->setGeometry(w->geometry());
        gameWindow->show();

        QWidget *top = this->window();
        top->close();*/
    }

}


/**
 * @brief Extrae el token de autenticación desde el archivo de configuración.
 * @param userKey Clave del usuario usada para formar la ruta del .conf.
 * @return Cadena con el token, o vacía si ocurre un error.
 *
 * Abre el fichero `.conf` asociado, busca la línea `token=...`
 * y devuelve su valor.
 */
QString RejoinWindow::loadAuthToken(const QString &userKey) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);

    qDebug() << "userKey: " << userKey;

    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Pon un qDebug() o algo para saber que falló
        qDebug() << "No se pudo abrir el archivo de configuración en MenuWindow." << configFile.fileName();
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
    return token;
}

/**
 * @brief Inicia la reconexión a la sala seleccionada.
 * @param idPart Identificador de la sala (string).
 *
 * Crea y configura un QWebSocket con el token y el id_partida,
 * conecta las señales y abre la conexión al servidor.
 */
void RejoinWindow::rejoin(QString idPart) {

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
}
