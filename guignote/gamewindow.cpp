#include "gamewindow.h"
#include "gamemessagewindow.h"
#include "settingswindow.h"
#include "menuwindow.h"
#include "carta.h"
#include "mano.h"
#include "deck.h"
#include "posicion.h"
#include <QLabel>
#include <QPixmap>
#include <QSize>
#include <QTransform>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QTimer>
#include <QtWebSockets>

QMap<QString, Carta*> GameWindow::cartasPorId;

void GameWindow::addCartaPorId(Carta* c){
    cartasPorId[c->idGlobal] = c;
}

Carta* GameWindow::getCartaPorId(QString id){
    return cartasPorId.value(id, nullptr);
}

GameWindow::GameWindow(int type, int fondo, QJsonObject msg, int id, QWebSocket *ws) {
    bg = fondo;
    gameType = type;
    player_id=id;
    cardSize = 175;
    this->ws = ws;
    QObject::connect(ws, &QWebSocket::textMessageReceived, this, &GameWindow::recibirMensajes);
    token = loadAuthToken();
    setBackground();
    setupUI();
    setupGameElements(msg);

    // Generar gameID
    if (msg.contains("gameID") && msg["gameID"].isString()) {
        gameID = msg["gameID"].toString();
        qDebug() << "GameWindow: usar gameID del servidor =" << gameID;
    } else {
        gameID = QUuid::createUuid().toString();
        qDebug() << "GameWindow: gameID generado localmente =" << gameID;
    }
}

void GameWindow::setupUI() {
    optionsBar = new QFrame(this);
    optionsBar->setObjectName("optionsBar");

    settings = new Icon(this);
    settings->setImage(":/icons/settings.png", 50, 50);
    chat = new Icon(this);
    chat->setImage(":/icons/message.png", 50, 50);
    quit = new Icon(this);
    quit->setImage(":/icons/door.png", 60, 60);

    connect(settings, &Icon::clicked, [=]() {
        settings->setImage(":/icons/darkenedsettings.png", 50, 50);
        SettingsWindow *settingsWin = new SettingsWindow(this, this);
        settingsWin->setModal(true);
        connect(settingsWin, &QDialog::finished, [this](int){
            settings->setImage(":/icons/settings.png", 50, 50);
        });
        settingsWin->exec();
    });

    connect(chat, &Icon::clicked, this, [this]() {
        qDebug() << "Icono de chat pulsado. Abriendo GameMessageWindow...";
        // Creamos la ventana de chat, pasándole el ID de partida y el ID de jugador:
        GameMessageWindow *chatWin =
            new GameMessageWindow(this,
                                  gameID,
                                  QString::number(player_id));
        chatWin->setWindowModality(Qt::ApplicationModal);
        // Centrar sobre la ventana padre:
        chatWin->move(this->geometry().center() - chatWin->rect().center());
        chatWin->show();
        chatWin->raise();
        chatWin->activateWindow();
    });

    connect(quit, &Icon::clicked, this, [this]() {
        quit->setImage(":/icons/darkeneddoor.png", 60, 60);
        QDialog *confirmDialog = new QDialog(this);
        confirmDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        confirmDialog->setModal(true);
        confirmDialog->setFixedSize(300,150);
        confirmDialog->setStyleSheet("QDialog { background-color: #171718; border-radius: 5px; padding: 20px; }");

        QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(confirmDialog);
        dialogShadow->setBlurRadius(10);
        dialogShadow->setColor(QColor(0, 0, 0, 80));
        dialogShadow->setOffset(4, 4);
        confirmDialog->setGraphicsEffect(dialogShadow);

        QVBoxLayout *dialogLayout = new QVBoxLayout(confirmDialog);
        QLabel *confirmLabel = new QLabel("¿Está seguro que desea salir?", confirmDialog);
        confirmLabel->setStyleSheet("QFrame { background-color: #171718; color: white; border-radius: 5px; }");
        confirmLabel->setAlignment(Qt::AlignCenter);
        dialogLayout->addWidget(confirmLabel);

        QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
        QString buttonStyle =
            "QPushButton {"
            "  background-color: #c2c2c3;"
            "  color: #171718;"
            "  border-radius: 15px;"
            "  font-size: 20px;"
            "  font-weight: bold;"
            "  padding: 12px 25px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #9b9b9b;"
            "}";
        QPushButton *yesButton = new QPushButton("Sí", confirmDialog);
        QPushButton *noButton = new QPushButton("No", confirmDialog);
        yesButton->setStyleSheet(buttonStyle);
        noButton->setStyleSheet(buttonStyle);
        yesButton->setFixedSize(100,40);
        noButton->setFixedSize(100,40);
        dialogButtonLayout->addWidget(yesButton);
        dialogButtonLayout->addWidget(noButton);
        dialogLayout->addLayout(dialogButtonLayout);
        connect(yesButton, &QPushButton::clicked, [this, confirmDialog]() {
            // cerramos el diálogo de confirmación
            confirmDialog->close();

            // creamos y mostramos el menú raíz
            QSize windowSize = this->size();
            MenuWindow *menuWindow = new MenuWindow();
            menuWindow->resize(windowSize);
            menuWindow->show();
            menuWindow->raise();
            menuWindow->activateWindow();

            // cerramos el GameWindow
            this->close();
        });
        connect(noButton, &QPushButton::clicked, [=]() {
            confirmDialog->close();
        });
        connect(confirmDialog, &QDialog::finished, [=](int) {
            quit->setImage(":/icons/door.png", 60, 60);
        });
        confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        confirmDialog->show();

        // QTimer para mantener el diálogo centrado
        QTimer *centerTimer = new QTimer(confirmDialog);
        centerTimer->setInterval(50);
        connect(centerTimer, &QTimer::timeout, [this, confirmDialog]() {
            confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        });
        centerTimer->start();
    });
}

void GameWindow::setBackground() {
    QString ornament;
    switch (bg) {
    case 0:
        ornament = ":/images/set-golden-border-ornaments/gold_ornaments.png";
        // Set background to the original green gradient pattern
        this->setStyleSheet(R"(
        /* Fondo de la ventana con gradiente verde */
        QWidget {
            background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                        fx:0.5, fy:0.5,
                                        stop:0 #1f5a1f,
                                        stop:1 #0a2a08);
        }

    /* Barra con gradiente vertical gris–negro */
    QFrame#optionsBar {
        background: qlineargradient(
            spread: pad,
            x1: 0, y1: 0,
            x2: 0, y2: 1,
            stop: 0 #3a3a3a,
            stop: 1 #000000
        );
        border-radius: 8px;
        border: 2px solid #000000;
    }
    )");
        break;
    case 1:
        ornament = ":/images/set-golden-border-ornaments/black_ornaments.png";
        // Set background to a red version of the pattern
        this->setStyleSheet(R"(
        /* Fondo de la ventana con gradiente rojo */
        QWidget {
            background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                        fx:0.5, fy:0.5,
                                        stop:0 #5a1f1f,  /* Red start color */
                                        stop:1 #2a0808); /* Dark red end color */
        }

    /* Barra con gradiente vertical gris–negro */
    QFrame#optionsBar {
        background: qlineargradient(
            spread: pad,
            x1: 0, y1: 0,
            x2: 0, y2: 1,
            stop: 0 #3a3a3a,
            stop: 1 #000000
        );
        border-radius: 8px;
        border: 2px solid #000000;
    }
    )");
        break;
    default:
        ornament = ":/images/set-golden-border-ornaments/gold_ornaments.png";
        // Set background to the original green gradient pattern
        this->setStyleSheet(R"(
        /* Fondo de la ventana con gradiente verde */
        QWidget {
            background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                        fx:0.5, fy:0.5,
                                        stop:0 #1f5a1f,
                                        stop:1 #0a2a08);
        }

    /* Barra con gradiente vertical gris–negro */
    QFrame#optionsBar {
        background: qlineargradient(
            spread: pad,
            x1: 0, y1: 0,
            x2: 0, y2: 1,
            stop: 0 #3a3a3a,
            stop: 1 #000000
        );
        border-radius: 8px;
        border: 2px solid #000000;
    }
    )");
        break;
    }

        QPixmap ornamentPixmap(ornament);
        // Decoraciones Esquinas
        ornamentSize = QSize(300, 299);

        cornerTopLeft = new QLabel(this);
        cornerTopRight = new QLabel(this);
        cornerBottomLeft = new QLabel(this);
        cornerBottomRight = new QLabel(this);

        cornerTopLeft->setPixmap(ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QTransform transformH;
        transformH.scale(-1, 1);
        cornerTopRight->setPixmap(
            ornamentPixmap.transformed(transformH, Qt::SmoothTransformation)
                .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );

        QTransform transformV;
        transformV.scale(1, -1);
        cornerBottomLeft->setPixmap(
            ornamentPixmap.transformed(transformV, Qt::SmoothTransformation)
                .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );

        QTransform transformHV;
        transformHV.scale(-1, -1);
        cornerBottomRight->setPixmap(
            ornamentPixmap.transformed(transformHV, Qt::SmoothTransformation)
                .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );

        QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
        for (QLabel* corner : corners) {
            corner->setFixedSize(ornamentSize);
            corner->setAttribute(Qt::WA_TransparentForMouseEvents);
            corner->setAttribute(Qt::WA_TranslucentBackground);
            corner->setStyleSheet("background: transparent;");
            corner->raise();
        }
}

void GameWindow::setupGameElements(QJsonObject msg) {
    manos.append(new Mano(0, 0));
    manos.append(new Mano(1, 1));
    posiciones.append(new Posicion(this, this, cardSize, 0, token, ws));
    posiciones.append(new Posicion(this, this, cardSize, 1, token, ws));

    if (gameType == 2) {
        manos.append(new Mano(2, 2));
        manos.append(new Mano(3, 3));
        posiciones.append(new Posicion(this, this, cardSize, 2, token, ws));
        posiciones.append(new Posicion(this, this, cardSize, 3, token, ws));
    }
    deck = new Deck(nullptr, 0, cardSize, this, token);

    setupGameState(msg);
}

void GameWindow::repositionHands(){
    for (Mano* mano : manos) mano->mostrarMano();
}

void GameWindow::repositionOrnaments() {
    int w = this->width();
    int h = this->height();
    int topOffset = 0;

    cornerTopLeft->move(0, topOffset);
    cornerTopRight->move(w - cornerTopRight->width(), topOffset);
    cornerBottomLeft->move(0, h - cornerBottomLeft->height());
    cornerBottomRight->move(w - cornerBottomRight->width(), h - cornerBottomRight->height());

    QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
    for (QLabel* corner : corners) {
        corner->lower();
    }
}

void GameWindow::repositionOptions() {
    // Set optionsBar position to the top-right corner of the screen
    int windowWidth = this->width();
    int windowHeight = this->height();

    int x = (windowWidth - windowWidth/8) / 2;  // Center horizontally

    // Place optionsBar at the top-right of the window with a height of 80
    optionsBar->setGeometry(x, 0, windowWidth/8, 80);
    optionsBar->raise();
    settings->raise();
    chat->raise();
    quit->raise();

    // Create a horizontal layout to arrange the icons inside optionsBar
    QHBoxLayout *layout = new QHBoxLayout(optionsBar);

    // Set a minimum space of 5 pixels between icons
    layout->setSpacing(5);

    // Add the three icons to the layout
    layout->addWidget(settings);
    layout->addWidget(chat);
    layout->addWidget(quit);

    // Make the icons stretch to fill the available space
    layout->setStretch(0, 1);
    layout->setStretch(1, 1);
    layout->setStretch(2, 1);

    // Set the layout to the optionsBar
    optionsBar->setLayout(layout);

    // Optionally, adjust the icons' alignment (optional, depending on preference)
    layout->setAlignment(Qt::AlignCenter);  // Align icons in the center horizontally
}

// Función para recolocar y reposicionar todos los elementos
void GameWindow::resizeEvent(QResizeEvent *event) {
    // Simply call QWidget's resizeEvent method
    QWidget::resizeEvent(event);
    repositionOrnaments();
    repositionHands();
    repositionOptions();
    deck->actualizarVisual();

    for (int i = 0; i < posiciones.size(); ++i) {
        posiciones[i]->mostrarPosicion();
    }
}

// FUNCIONES PARA COMUNICARSE CON BACKEND Y JUGAR PARTIDAS

// Función auxiliar para crear un diálogo modal con mensaje personalizado.
// Si exitApp es verdadero, al cerrar se finaliza la aplicación.
static QDialog* createDialog(QWidget *parent, const QString &message, bool exitApp = false) {
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog->setStyleSheet("QDialog { background-color: #171718; border-radius: 5px; padding: 20px; }");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialog);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(4, 4);
    dialog->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QLabel *label = new QLabel(message, dialog);
    label->setWordWrap(true);
    label->setStyleSheet("color: white; font-size: 16px;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QPushButton *okButton = new QPushButton("OK", dialog);
    okButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; color: #171718; border-radius: 15px;"
        "font-size: 20px; font-weight: bold; padding: 12px 25px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    okButton->setFixedSize(100, 40);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    QObject::connect(okButton, &QPushButton::clicked, [dialog, exitApp]() {
        dialog->close();
        if (exitApp)
            qApp->quit();
    });

    dialog->adjustSize();
    dialog->move(parent->geometry().center() - dialog->rect().center());
    return dialog;
}

// Función para extraer el token de autenticación desde el archivo .conf
QString GameWindow::loadAuthToken() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + "/Grace Hopper/Sota, Caballo y Rey.conf";
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        createDialog(this, "No se pudo cargar el archivo de configuración.")->show();
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
        createDialog(this, "No se encontró el token en el archivo de configuración.")->show();
    }
    return token;
}

/* Setup:
 * Deck:
 *  numCartas
 *  cartaTriunfo
 * Mano Mia:
 *  Asignar my id
 *  Añadir mis cartas a la mano
 * Mano Resto:
 *  Añadir sus ids
 *  Añadir n backs a la mano
 */
void GameWindow::setupGameState(QJsonObject s0){

    // Comprobamos que tenemos el id

    // Inicializamos deck
    int mazoRestante = s0.value("mazo_restante").toInt();
    QJsonObject cartaTriunfo = s0.value("carta_triunfo").toObject();
    QString palo = cartaTriunfo.value("palo").toString();
    QString valor = QString::number(cartaTriunfo.value("valor").toInt());

    qDebug() << "Valor Triunfo " << valor;
    qDebug() << "Palo Triunfo " << palo;

    deck->setNum(mazoRestante);
    Carta *cartaT = new Carta (this, this, valor, palo, cardSize, 0);
    deck->setTriunfo(cartaT);

    // Inicializamos mi mano
    manos[0]->player_id = this->player_id;
    posiciones[0]->player_id = this->player_id;

    QJsonArray misCartasArray = s0.value("mis_cartas").toArray();
    QString val;
    for (const QJsonValue& cartaVal : misCartasArray) {
        QJsonObject cartaObj = cartaVal.toObject();
        QString palo = cartaObj.value("palo").toString();
        val = QString::number(cartaObj.value("valor").toInt());

        // Aquí puedes crear una instancia de tu clase Carta
        Carta* carta = new Carta(this, this, val, palo, cardSize, 0);

        // Y agregarla a la mano, por ejemplo:
        manos[0]->añadirCarta(carta);
    }

    // Inicializamos Resto de Jugadores
    QJsonArray jugadoresArray = s0.value("jugadores").toArray();

    int i = 0;
    int posiciones[] = {2, 1, 3};
    int pos;

    for (const QJsonValue& jugadorVal : jugadoresArray) {
        QJsonObject jugadorObj = jugadorVal.toObject();

        int id = jugadorObj.value("id").toInt();
        QString nombre = jugadorObj.value("nombre").toString();
        int equipo = jugadorObj.value("equipo").toInt();
        int numCartas = jugadorObj.value("num_cartas").toInt();
        if (id != player_id) {

            if(gameType == 2){
                pos = posiciones[i];
            } else {
                pos = 1;
            }

            manos[pos]->player_id = id;
            this->posiciones[pos]->player_id = id;
            for(int j = 0; j < numCartas; j++){
                Carta* back = new Carta(this, this, "0", "", cardSize, 0);
                manos[pos]->añadirCarta(back);
            }
            i++;
        }
    }
}

void GameWindow::recibirMensajes(const QString &mensaje) {
    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();
    QJsonObject data = obj.value("data").toObject();

    if (type == "turn_update") {
        QString msg = data.value("message").toString();
        int jugadorId = data["jugador"].toObject()["id"].toInt();
        QString nombre = data["jugador"].toObject()["nombre"].toString();
        int turnoIndex = data["turno_index"].toInt();
        qDebug() << "Turno de" << nombre << "(ID:" << jugadorId << ") - Index:" << turnoIndex;

        // Permitimos jugar cartas solo en tu turno
        if(jugadorId == player_id){
            posiciones[0]->setLock(false);
            qDebug() << "UnLocked";
        } else {
            posiciones[0]->setLock(true);
            qDebug() << "Locked";
        }
    }
    else if (type == "card_played") {
        int jugadorId = data["jugador"].toObject()["id"].toInt();
        QString nombre = data["jugador"].toObject()["nombre"].toString();
        bool automatica = data["automatica"].toBool();
        QString palo = data["carta"].toObject()["palo"].toString();
        int valor = data["carta"].toObject()["valor"].toInt();
        qDebug() << nombre << "jugó" << valor << "de" << palo << (automatica ? "(automática)" : "");
    }
    else if (type == "round_result") {
        QString ganador = data["ganador"].toObject()["nombre"].toString();
        int puntos = data["puntos_baza"].toInt();
        int puntos1 = data["puntos_equipo_1"].toInt();
        int puntos2 = data["puntos_equipo_2"].toInt();
        qDebug() << "Ganador de la ronda:" << ganador << "- Puntos:" << puntos << "→ E1:" << puntos1 << "E2:" << puntos2;

        for (Posicion* pos : posiciones) {
            if (pos) {
                // Usás el puntero directamente
                pos->removeCard();  // por ejemplo
            }
        }

    }
    else if (type == "phase_update") {
        QString msg = data.value("message").toString();
        qDebug() << "[Fase de arrastre]" << msg;
    }
    else if (type == "card_drawn") {
        QString palo = data["carta"].toObject()["palo"].toString();
        int valor = data["carta"].toObject()["valor"].toInt();
        qDebug() << "Robaste una carta:" << valor << "de" << palo;
    }
    else if (type == "player_left") {
        QString msg = data["message"].toString();
        QString nombre = data["usuario"].toObject()["nombre"].toString();
        qDebug() << nombre << "se desconectó." << msg;
    }
    else if (type == "end_game") {
        QString msg = data["message"].toString();
        int ganadorEquipo = data["ganador_equipo"].toInt();
        int puntos1 = data["puntos_equipo_1"].toInt();
        int puntos2 = data["puntos_equipo_2"].toInt();
        qDebug() << "🏁 Fin del juego:" << msg << "Ganador: Equipo" << ganadorEquipo << "→ E1:" << puntos1 << "E2:" << puntos2;
    }
    else {
        qDebug() << "Mensaje desconocido:" << mensaje;
    }
}
