/**
 * @file rankingwindow.cpp
 * @brief Implementación de la clase RankingWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Este archivo define la ventana de rankings, que muestra listas
 * de puntuaciones individuales o por parejas, con opción de filtrar
 * solo amigos. Se encarga de la UI, de obtener datos del servidor
 * y de actualizar la lista.
 */

#include "rankingwindow.h"

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

/**
 * @brief Constructor de RankingWindow.
 * @param userKey Clave de usuario para cargar el token.
 * @param parent Widget padre (por defecto nullptr).
 *
 * Configura la ventana como dialog sin bordes, carga el token,
 * construye la interfaz y lanza la petición inicial para el ranking individual.
 */
RankingWindow::RankingWindow(const QString &userKey, QWidget *parent)
    : QDialog(parent),
      currentUserName(userKey)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(900, 650);

    networkManager = new QNetworkAccessManager(this);
    authToken = loadAuthToken(userKey);
    setupUI();
    fetchIndividualRanking();
}

/**
 * @brief Carga el token de autenticación desde el archivo .conf del usuario.
 * @param userKey Clave de usuario usada en el nombre del archivo.
 * @return QString Token extraído o cadena vacía si falla.
 */
QString RankingWindow::loadAuthToken(const QString &userKey) {
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
 * @brief Construye y organiza todos los widgets de la UI.
 *
 * Crea layouts, cabecera, botones de selección, checkbox de “Sólo amigos”,
 * la lista de rankings y conecta señales de los botones.
 */
void RankingWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Rankings", this);
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

    individualButton = new QPushButton("Individual", this);
    parejasButton = new QPushButton("Parejas", this);
    soloAmigosCheck = new QCheckBox("Solo amigos", this);

    connect(soloAmigosCheck, &QCheckBox::checkStateChanged, this, [=](int state){
        amigos = (state == Qt::Checked) ? "_amigos" : "";
        if (lastPressed == 1) {
            fetchIndividualRanking();
        } else if (lastPressed == 2) {
            fetchTeamRanking();
        }
    });

    QString buttonStyle =
        "QPushButton {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   padding: 12px 25px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #9b9b9b;"
        "}";

    QString checkboxStyle =
        "QCheckBox {"
        "   color: #ffffff;"
        "   font-size: 14px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 16px;"
        "   height: 16px;"
        "}"
        "QCheckBox::indicator:unchecked {"
        "   background-color: #c2c2c3;"
        "   border: 1px solid #545454;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color: #c2c2c3;"
        "   border: 1px solid #545454;"
        "   image: url(:/icons/cross.png);"
        "}";

    individualButton->setStyleSheet(buttonStyle);
    parejasButton->setStyleSheet(buttonStyle);
    soloAmigosCheck->setStyleSheet(checkboxStyle);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(individualButton);
    buttonsLayout->addSpacing(15);
    buttonsLayout->addWidget(parejasButton);

    QWidget *buttonsContainer = new QWidget(this);
    buttonsContainer->setLayout(buttonsLayout);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addStretch();
    filterLayout->addWidget(buttonsContainer);
    filterLayout->addSpacing(15);
    filterLayout->addWidget(soloAmigosCheck);
    filterLayout->addStretch();

    mainLayout->addLayout(filterLayout);

    rankingListWidget = new QListWidget(this);

    rankingListWidget->setStyleSheet(R"(
    rankingListWidget->setStyleSheet(R"(
    QListWidget {
        background: transparent;
        border: none;
    }

    /* Eliminamos el reborde punteado que dibuja Qt cuando hay foco/selección */
    QListWidget::item:selected {
        outline: none;
    }
    )");

    rankingListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    rankingListWidget->setMinimumWidth(600);
    rankingListWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    rankingListWidget->setTextElideMode(Qt::ElideNone);
    rankingListWidget->setWordWrap(true);
    rankingListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rankingListWidget->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *centeredListLayout = new QHBoxLayout();
    centeredListLayout->addStretch();
    centeredListLayout->addWidget(rankingListWidget);
    centeredListLayout->addStretch();
    mainLayout->addLayout(centeredListLayout);

    connect(individualButton, &QPushButton::clicked, this, &RankingWindow::fetchIndividualRanking);
    connect(parejasButton, &QPushButton::clicked, this, &RankingWindow::fetchTeamRanking);
}

/**
 * @brief Solicita al servidor el ranking individual.
 *
 * Monta la URL según si está marcado “Solo amigos” y envía un GET.
 * Conecta la respuesta a @ref handleIndividualRankingResponse.
 */
void RankingWindow::fetchIndividualRanking() {
    QString cat = "top_elo" + amigos;
    QUrl url(QString("http://188.165.76.134:8000/usuarios/" + cat +"/"));
    QNetworkRequest request(url);
    request.setRawHeader("Auth", authToken.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &RankingWindow::handleIndividualRankingResponse);
    lastPressed = 1;
}

/**
 * @brief Solicita al servidor el ranking por parejas.
 *
 * Monta la URL correspondiente (parejas + filtro amigos) y envía un GET.
 * Conecta la respuesta a @ref handleTeamRankingResponse.
 */
void RankingWindow::fetchTeamRanking() {
    QString cat = "top_elo_parejas" + amigos;
    QUrl url(QString("http://188.165.76.134:8000/usuarios/" + cat +"/"));
    QNetworkRequest request(url);
    request.setRawHeader("Auth", authToken.toUtf8());
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &RankingWindow::handleTeamRankingResponse);
    lastPressed = 2;
}

/**
 * @brief Procesa la respuesta del servidor para el ranking individual.
 *
 * Si no hay error, parsea el JSON y extrae el array "top_elo_players",
 * luego llama a @ref updateRankingList con dicho array.
 * En caso de error, imprime en debug.
 */
void RankingWindow::handleIndividualRankingResponse() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonArray playersArray = jsonDoc.object().value("top_elo_players").toArray();
        updateRankingList(playersArray, 1);
    } else {
        qDebug() << "Error al obtener el ranking individual:" << reply->errorString();
    }
    reply->deleteLater();
}

/**
 * @brief Procesa la respuesta del servidor para el ranking por parejas.
 *
 * Si no hay error, parsea el JSON y extrae el array "top_elo_parejas_players",
 * luego llama a @ref updateRankingList con dicho array.
 * En caso de error, imprime en debug.
 */
void RankingWindow::handleTeamRankingResponse() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonArray playersArray = jsonDoc.object().value("top_elo_parejas_players").toArray();
        updateRankingList(playersArray, 2);
    } else {
        qDebug() << "Error al obtener el ranking por parejas:" << reply->errorString();
    }
    reply->deleteLater();
}

/**
 * @brief Actualiza el QListWidget con los datos del ranking.
 * @param playersArray Array de objetos JSON con campos "nombre" y "elo"/"elo_parejas".
 * @param type       1=individual, 2=parejas.
 *
 * Limpia la lista, recorre el array, crea items con formato,
 * aplica “zebra” y resalta al usuario actual.
 */
void RankingWindow::updateRankingList(const QJsonArray &playersArray, int type)
{
    rankingListWidget->clear();

    int position = 1;
    for (const QJsonValue &value : playersArray) {
        QJsonObject playerObj = value.toObject();
        QString playerName = playerObj.value("nombre").toString();

        int elo = (type == 1)
                      ? playerObj.value("elo").toInt()
                      : playerObj.value("elo_parejas").toInt();

        QString listItemText = QString("%1º  %2 - Elo: %3")
                                   .arg(position)
                                   .arg(playerName)
                                   .arg(elo);

        /* ────────────── Crear item ────────────── */
        QListWidgetItem *item = new QListWidgetItem(listItemText);

        /* --- tipografía base --- */
        QFont font("Montserrat", 15);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 0.6);
        item->setFont(font);
        item->setSizeHint(QSize(rankingListWidget->width() - 40, 40));

        /* >>> NUEVA LÍNEA: color de texto por defecto <<< */
        item->setForeground(QColor("#ffffff"));   // texto blanco

        /* --- zebra manual (posición empieza en 1) --- */
        if (position % 2 == 1) {                               // filas 1,3,5…
            item->setBackground(QColor(255, 255, 255, 15));    // ~6 % opacidad
        }

        /* --- resaltar a mi usuario --- */
        if (playerName == currentUserName) {
            item->setBackground(QColor("#3d82f6"));
            item->setForeground(QColor("#ffffff"));            // texto casi negro
            font.setBold(true);
            item->setFont(font);
        }

        rankingListWidget->addItem(item);
        ++position;
    }

    rankingListWidget->updateGeometry();
}

