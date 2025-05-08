#include "customgameswindow.h"
#include "crearcustomgame.h"

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

    soloAmigosCheck = new QCheckBox("Solo amigos", this);
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

void CustomGamesWindow::fetchAllGames(){
    qDebug() << "fetching all games";
}

void CustomGamesWindow::fetchFriendGames(){
    qDebug() << "fetching friend only games";
}
