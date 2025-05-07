#include "rejoinwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWebSocket>
#include "gamewindow.h"

RejoinWindow::RejoinWindow(QJsonArray jsonArray, int type, int fondo, QString &userKey, QString usr, QWidget *parent)
    : QDialog(parent), salas(jsonArray) {
    this->type = type;
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

void RejoinWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(createHeaderLayout());
    populateSalas();

    setLayout(mainLayout);
}

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

        int numJugadores = jugadores.size();

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

        // — Construimos el GameWindow y lo colocamos exactamente donde estaba el menú —
        GameWindow *gameWindow = new GameWindow(userKey, type, 1, data, id, webSocket, usr, menuWin);
        // Le damos la misma posición y tamaño que el MenuWindow
        gameWindow->setGeometry(w->geometry());
        gameWindow->show();

        QWidget *top = this->window();
        top->close();
    }
}


// Función para extraer el token de autenticación desde el archivo .conf
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
