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
#include <QGraphicsDropShadowEffect>
#include <QApplication>

CrearCustomGame::CrearCustomGame(QString &userKey, QString usr, int fondo, QDialog *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #222; border-radius: 30px; padding: 20px;");
    setFixedSize(500, 450);

    this->userKey = userKey;
    this->usr = usr;
    this->fondo = fondo;
    networkManager = new QNetworkAccessManager(this);

    token = loadAuthToken(userKey);
    setupUI();
}

void CrearCustomGame::setupUI(){
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // Estilo de checkbox
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
    connect(closeButton, &QPushButton::clicked, this, &CrearCustomGame::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // ——— “Solo amigos” ———
    QHBoxLayout *amigosLayout = new QHBoxLayout();
    amigosLayout->addWidget(new QLabel("Solo amigos", this));
    soloAmigos = new QCheckBox(this);
    soloAmigos->setChecked(soloAmigosB);
    connect(soloAmigos, &QCheckBox::stateChanged, this, [this](int state){
        soloAmigosB = (state == Qt::Checked);
    });
    amigosLayout->addWidget(soloAmigos);
    mainLayout->addLayout(amigosLayout);

    // ——— “Capacidad” ———
    QHBoxLayout *capacidadLayout = new QHBoxLayout();
    capacidadLayout->addWidget(new QLabel("Capacidad", this));
    ind = new QCheckBox("Individual", this);
    par = new QCheckBox("Parejas", this);
    ind->setChecked(individual);
    par->setChecked(!individual);
    connect(ind, &QCheckBox::stateChanged, this, [this](int s){
        if (s == Qt::Checked) {
            individual = true;
            par->setChecked(false);
        }
    });
    connect(par, &QCheckBox::stateChanged, this, [this](int s){
        if (s == Qt::Checked) {
            individual = false;
            ind->setChecked(false);
        }
    });


    // ——— “Tiempo por turno” ———
    QHBoxLayout *tiempoLayout = new QHBoxLayout();
    tiempoLayout->addWidget(new QLabel("Tiempo por turno", this));
    t15s = new QCheckBox("15s", this);
    t30s = new QCheckBox("30s", this);
    t60s = new QCheckBox("60s", this);
    // estados iniciales
    t15s->setChecked(tiempo == 15);
    t30s->setChecked(tiempo == 30);
    t60s->setChecked(tiempo == 60);
    // comportarse como “radio”:
    connect(t15s, &QCheckBox::stateChanged, this, [this](int s){
        if (s == Qt::Checked) {
            tiempo = 15;
            t30s->setChecked(false);
            t60s->setChecked(false);
        }
    });
    connect(t30s, &QCheckBox::stateChanged, this, [this](int s){
        if (s == Qt::Checked) {
            tiempo = 30;
            t15s->setChecked(false);
            t60s->setChecked(false);
        }
    });
    connect(t60s, &QCheckBox::stateChanged, this, [this](int s){
        if (s == Qt::Checked) {
            tiempo = 60;
            t15s->setChecked(false);
            t30s->setChecked(false);
        }
    });
    tiempoLayout->addWidget(t15s);
    tiempoLayout->addWidget(t30s);
    tiempoLayout->addWidget(t60s);
    mainLayout->addLayout(tiempoLayout);

    // ——— “Revueltas” ———
    QHBoxLayout *revLayout = new QHBoxLayout();
    revLayout->addWidget(new QLabel("Revueltas", this));
    revueltas = new QCheckBox(this);
    revueltas->setChecked(revueltasB);
    connect(revueltas, &QCheckBox::stateChanged, this, [this](int state){
        revueltasB = (state == Qt::Checked);
    });
    revLayout->addWidget(revueltas);
    mainLayout->addLayout(revLayout);

    // ——— “Fase de arrastre” ———
    QHBoxLayout *arrastreLayout = new QHBoxLayout();
    arrastreLayout->addWidget(new QLabel("Fase de arrastre", this));
    arrastre = new QCheckBox(this);
    arrastre->setChecked(arrastreB);
    connect(arrastre, &QCheckBox::stateChanged, this, [this](int state){
        arrastreB = (state == Qt::Checked);
    });
    arrastreLayout->addWidget(arrastre);
    mainLayout->addLayout(arrastreLayout);

    // ——— Botón “Crear” ———
    QPushButton *crearBtn = new QPushButton("Crear", this);
    crearBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #45A049; }"
        );
    connect(crearBtn, &QPushButton::clicked, this, [this](){
        qDebug() << "soloAmigosB:" << soloAmigosB
                 << "tiempo:"      << tiempo
                 << "revueltasB:"  << revueltasB
                 << "arrastreB:"   << arrastreB;
        crearPartida();
    });
    mainLayout->addWidget(crearBtn, 0, Qt::AlignCenter);

    soloAmigos->setStyleSheet(checkboxStyle);
    t15s->setStyleSheet(checkboxStyle);
    t30s->setStyleSheet(checkboxStyle);
    t60s->setStyleSheet(checkboxStyle);
    revueltas->setStyleSheet(checkboxStyle);
    arrastre->setStyleSheet(checkboxStyle);
}

QString CrearCustomGame::loadAuthToken(const QString &userKey) {
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

void CrearCustomGame::crearPartida(){

    // Inicializamos contadores
    int capacidad;

    if (individual) { capacidad = 2; } else { capacidad = 4; }

    jugadoresCola = 1;
    jugadoresMax  = capacidad;

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
ws://188.165.76.134:8000/ws/partida/?token=TU_TOKEN&es_personalizada=true&capacidad=4&solo_amigos=true&tiempo_turno=60&permitir_revueltas=true&reglas_arrastre=true
    QString url = QString("ws://188.165.76.134:8000/ws/partida/?token=%1&capacidad=%2&solo_amigos=%3&tiempo_turno=%4&permitir_revueltas=%5&reglas_arrastre=%6")
                      .arg(token)
                      .arg(capacidad)
                      .arg(soloAmigosB)
                      .arg(tiempo)
                      .arg(revueltasB)
                      .arg(arrastreB);
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

    // contador (1/Capacidad)
    countLabel = new QLabel(
        QString("(%1/%2)").arg(jugadoresCola).arg(jugadoresMax),
        searchingDialog
        );
    countLabel->setStyleSheet("color: white; font-size: 20px; background: transparent;");
    countLabel->setAlignment(Qt::AlignCenter);
    searchLayout->addWidget(countLabel);

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

void CrearCustomGame::manejarMensaje(const QString &userKey, const QString &mensaje) {
    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "❌ Mensaje no es JSON válido.";
        return;
    }

    QJsonObject root = doc.object();
    QString tipo = root.value("type").toString();
    QJsonObject data = root.value("data").toObject();

    if (tipo == "player_joined") {
        QString nombre = data["usuario"].toObject()["nombre"].toString();
        int id = data["usuario"].toObject()["id"].toInt();
        int chatId = data["chat_id"].toInt();
        jugadoresCola = data.value("jugadores").toInt();
        jugadoresMax = data.value("capacidad").toInt();

        qDebug() << "ChatID: " << chatId;
        qDebug() << "(" << jugadoresCola << "/" << jugadoresMax << ")";

        // Actualizamos el contador en el diálogo
        if (searchingDialog && countLabel) {
            countLabel->setText(
                QString("(%1/%2)").arg(jugadoresCola).arg(jugadoresMax)
                );
        }

        qDebug() << "👤 Se ha unido un jugador:" << nombre << "(ID:" << id << ")";

        if(nombre == usr){
            this->id = id;
        }

        if (mensajeCola) {
            QString nuevoTexto = QString("Esperando en cola... (%1/%2)")
            .arg(jugadoresCola)
                .arg(jugadoresMax);
            mensajeCola->setText(nuevoTexto);
            mensajeCola->adjustSize();

            // Recalcular posición centrada
            int x = (this->width() - mensajeCola->width()) / 2;
            int y = this->height() - mensajeCola->height() - 80;
            mensajeCola->move(x, y);
            mensajeCola->raise();
        }
    }

    else if (tipo == "start_game") {
        // — cerrar cuadro “Buscando oponente” si está abierto —
        if (searchingDialog) {
            searchingDialog->close();
            searchingDialog->deleteLater();
            searchingDialog = nullptr;
        }
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

        // Get the current size of MenuWindow
        QSize windowSize = this->size();  // Get the size of MenuWindow


        // 1) Obtener widget padre y abuelo
        QWidget *padre   = parentWidget();
        QWidget *abuelo  = padre ? padre->parentWidget() : nullptr;

        // 2) Intentar hacer cast a MenuWindow
        MenuWindow *menu = qobject_cast<MenuWindow*>(abuelo);
        if (!menu) {
            qWarning() << "CrearCustomGame no tiene un abuelo de tipo MenuWindow!";
            return;
        }
        // — Construimos el GameWindow y lo colocamos exactamente donde estaba el menú —
        GameWindow *gameWindow = new GameWindow(userKey, type, fondo, data, id, webSocket, usr, menu);
        // Le damos la misma posición y tamaño que el MenuWindow
        gameWindow->setGeometry(menu->geometry());
        gameWindow->show();

        // Cierra todo top-level widget excepto nuestro gameWindow
        for (QWidget *w : QApplication::topLevelWidgets()) {
            if (w != gameWindow) {
                w->close();
            }
        }
    }
}

