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
#include <QGraphicsDropShadowEffect>
#include <QApplication>

CrearCustomGame::CrearCustomGame(QString &userKey, QString usr, int fondo, QDialog *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #222; border-radius: 30px; padding: 20px;");
    setFixedSize(650, 550);

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
    capacidadLayout->addWidget(ind);
    capacidadLayout->addWidget(par);
    mainLayout->addLayout(capacidadLayout);

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
    ind->setStyleSheet(checkboxStyle);
    par->setStyleSheet(checkboxStyle);
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


    QString url = QString("ws://188.165.76.134:8000/ws/partida/?token=%1&capacidad=%2&solo_amigos=%3&tiempo_turno=%4&permitir_revueltas=%5&reglas_arrastre=%6&es_personalizada=%7")
                      .arg(token)
                      .arg(capacidad)
                      .arg(soloAmigosB  ? "true" : "false")
                      .arg(tiempo)
                      .arg(revueltasB ? "true" : "false")
                      .arg(arrastreB  ? "true" : "false")
                      .arg("true");
    qDebug() << "Conectando a:" << url;

    // 1) Obtener widget padre y abuelo
    QWidget *padre   = parentWidget();
    QWidget *abuelo  = padre ? padre->parentWidget() : nullptr;

    // 2) Intentar hacer cast a MenuWindow
    MenuWindow *menu = qobject_cast<MenuWindow*>(abuelo);
    if (!menu) {
        qWarning() << "CrearCustomGame no tiene un abuelo de tipo MenuWindow!";
        return;
    }

    // Creamos la nueva ventana (EstadoPartida o GameWindow)
    EstadoPartida *gameWindow = new EstadoPartida(usr, userKey, menu, url, 1, 1, [this]() {
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
