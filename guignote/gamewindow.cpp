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
#include <QPoint>//


struct Slot {
    Posicion* posW;
    Carta*    c;
    QPoint    start;
};

QMap<QString, Carta*> GameWindow::cartasPorId;
static QDialog* createDialog(QWidget *parent, const QString &message, bool exitApp = false);

void GameWindow::addCartaPorId(Carta* c){
    cartasPorId[c->idGlobal] = c;
}

Carta* GameWindow::getCartaPorId(QString id){
    return cartasPorId.value(id, nullptr);
}

GameWindow::GameWindow(const QString &userKey, int type, int fondo, QJsonObject msg, int id, QWebSocket *ws) {
    bg = fondo;
    gameType = type;
    player_id=id;
    cardSize = 175;
    this->ws = ws;
    QObject::connect(ws, &QWebSocket::textMessageReceived, this, &GameWindow::recibirMensajes);
    token = loadAuthToken(userKey);
    setBackground();
     if (msg.contains("chat_id")) {
        chatID = QString::number(msg["chat_id"].toInt());
    }
    setupUI(userKey);
    setupGameElements(msg);
    setMouseTracking(true);
    optionsBar->setMouseTracking(true);

    if (msg.contains("gameID") && msg["gameID"].isString()) {
        gameID = msg["gameID"].toString();
    }
}

void GameWindow::setupUI(const QString &userKey) {

    // Registrar fuente personalizada
    int fontId = QFontDatabase::addApplicationFont(":/fonts/GlossypersonaluseRegular-eZL93.otf");
    QString fontFamily;
    if (fontId != -1) {
        fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        qDebug() << "✔ Fuente cargada:" << fontFamily;
    } else {
        qWarning() << "❌ No se pudo cargar la fuente Glossy Personaluse.";
        fontFamily = "Arial";  // Fuente alternativa
    }

    // 1) Barra de opciones y layout
    optionsBar = new QFrame(this);
    optionsBar->setObjectName("optionsBar");
    optionsBar->setFixedHeight(optionsBarHeight);

    auto *lay = new QHBoxLayout(optionsBar);
    lay->setSpacing(20);
    lay->setContentsMargins(15, 8, 15, 8);
    lay->setAlignment(Qt::AlignCenter);

    // 2) Crear los iconos UNA sola vez
    settings = new Icon(this);
    settings->setImage(":/icons/settings.png", 50, 50);
    chat = new Icon(this);
    chat->setImage(":/icons/message.png", 50, 50);
    quit = new Icon(this);
    quit->setImage(":/icons/door.png", 60, 60);

    // 3) Añadirlos al layout
    lay->addWidget(settings);
    lay->addWidget(chat);
    lay->addWidget(quit);

    // 4) Conectar sus señales
    connect(settings, &Icon::clicked, this, [=]() {
        settings->setImage(":/icons/darkenedsettings.png", 50, 50);
        SettingsWindow *w = new SettingsWindow(this, this);
        w->setModal(true);
        connect(w, &QDialog::finished, [=](int){ settings->setImage(":/icons/settings.png",50,50); });
        w->exec();
    });

    connect(chat, &Icon::clicked, this, [this, userKey]() {
        if (chatID.isEmpty()) {
            // aún no ha llegado start_game con chat_id
            QDialog *d = createDialog(this, "El chat no está disponible. Espera a que inicie la partida.");
            d->show();
            return;
        }
        GameMessageWindow *w = new GameMessageWindow(userKey, this, chatID, QString::number(player_id));
        w->setWindowModality(Qt::ApplicationModal);
        w->move(this->geometry().center() - w->rect().center());
        w->show(); w->raise(); w->activateWindow();
    });

    connect(quit, &Icon::clicked, this, [this, userKey]() {
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
        connect(yesButton, &QPushButton::clicked, [this, confirmDialog, userKey]() {
            // cerramos el diálogo de confirmación
            confirmDialog->close();

            // creamos y mostramos el menú raíz
            QSize windowSize = this->size();
            MenuWindow *menuWindow = new MenuWindow(userKey);
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
    // 5) Indicador siempre visible
    optionsIndicator = new QLabel(this);
    optionsIndicator->setObjectName("optionsIndicator");
    optionsIndicator->setFixedHeight(indicatorHeight);
    optionsIndicator->setStyleSheet(R"(
        QLabel#optionsIndicator {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #c2c2c3, stop:1 #9b9b9b);
            border-radius: 3px;
        }
    )");
    optionsIndicator->installEventFilter(this);

    // 6) Animaciones y temporizador
    showOptionsAnimation = new QPropertyAnimation(optionsBar, "pos", this);
    showOptionsAnimation->setDuration(300);
    showOptionsAnimation->setEasingCurve(QEasingCurve::OutQuad);

    hideOptionsAnimation = new QPropertyAnimation(optionsBar, "pos", this);
    hideOptionsAnimation->setDuration(300);
    hideOptionsAnimation->setEasingCurve(QEasingCurve::InQuad);

    hideOptionsTimer = new QTimer(this);
    hideOptionsTimer->setInterval(2000);
    connect(hideOptionsTimer, &QTimer::timeout, this, [=](){
        if (!isMouseOverOptions && isOptionsVisible)
            hideOptionsAnimation->start();
        isOptionsVisible = false;
    });

    // 7) Posicionar inicialmente y asegurar Z‑order
    repositionOptions();          // centra y dimensiona barra + indicador
    optionsBar->move(optionsBar->x(), -optionsBarHeight);
    optionsIndicator->move(
        optionsBar->x() + (optionsBar->width() - optionsIndicator->width())/2,
        0
        );
    optionsBar->raise();
    optionsIndicator->raise();
    hideOptionsTimer->start();

    // Capa oscura de overlay
    overlay = new QWidget(this);
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 200);");
    overlay->hide();

    // Efecto de opacidad + animaciones
    overlayEffect = new QGraphicsOpacityEffect(overlay);
    overlay->setGraphicsEffect(overlayEffect);
    overlayEffect->setOpacity(0.0);

    fadeIn = new QPropertyAnimation(overlayEffect, "opacity", this);
    fadeIn->setDuration(700);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutQuad);

    fadeOut = new QPropertyAnimation(overlayEffect, "opacity", this);
    fadeOut->setDuration(500);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InOutQuad);
    connect(fadeOut, &QPropertyAnimation::finished, [this]() {
        overlay->hide();
    });

    // Texto centrado
    turnoLabel = new QLabel(overlay);
    turnoLabel->setAlignment(Qt::AlignCenter);
    turnoLabel->setStyleSheet("color: white; font-size: 40px; background: transparent;");
    turnoLabel->setGeometry(overlay->rect());
    QFont font(fontFamily, 38);
    turnoLabel->setFont(font);
}




bool GameWindow::eventFilter(QObject *watched, QEvent *event) {
    if ((watched == optionsBar || watched == optionsIndicator)) {
        if (event->type() == QEvent::Enter) {
            isMouseOverOptions = true;
            if (!isOptionsVisible) {
                showOptionsAnimation->start();
                isOptionsVisible = true;
            }
            return true;
        }
        else if (event->type() == QEvent::Leave) {
            isMouseOverOptions = false;
            hideOptionsTimer->start();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}


void GameWindow::mostrarTurno(const QString &texto, bool /*miTurno*/) {
    turnoLabel->setText(texto);

    overlay->show();
    overlay->raise();
    optionsBar->raise();



    fadeOut->stop();
    fadeIn->start();

    for (Carta* carta : manos[0]->cartas)
        carta->raise();

    QTimer::singleShot(2500, this, &GameWindow::ocultarTurno);
}




void GameWindow::ocultarTurno() {
    fadeIn->stop();
    fadeOut->start();
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
    for (Posicion* posW : posiciones) {
        pilaCount[posW] = 0;
    }
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
    auto *lay = qobject_cast<QHBoxLayout*>(optionsBar->layout());
    if (!lay) return;

    int windowW = width();
    int iconsW   = settings->width() + chat->width() + quit->width();
    int spacing  = lay->spacing();
    QMargins m   = lay->contentsMargins();
    int minW     = iconsW + spacing*2 + m.left() + m.right();
    int barW     = qMax(windowW/8, minW);

    optionsBar->setFixedWidth(barW);
    int x = (windowW - barW) / 2;
    optionsBar->move(x, optionsBar->y());

    // Animaciones
    showOptionsAnimation->setStartValue(QPoint(x, -optionsBarHeight));
    showOptionsAnimation->setEndValue  (QPoint(x,    0));
    hideOptionsAnimation->setStartValue(QPoint(x,    0));
    hideOptionsAnimation->setEndValue  (QPoint(x, -optionsBarHeight));

    // Indicador proporcional
    int indW = barW / 3;
    optionsIndicator->setFixedSize(indW, indicatorHeight);
    optionsIndicator->move(
        x + (barW - indW)/2,
        0
        );

    // Siempre al frente
    optionsBar->raise();
    optionsIndicator->raise();
}




// Función para recolocar y reposicionar todos los elementos
void GameWindow::resizeEvent(QResizeEvent *event) {
    // Simply call QWidget's resizeEvent method
    QWidget::resizeEvent(event);
    repositionOrnaments();
    repositionHands();
    repositionOptions();
    deck->actualizarVisual();
    overlay->setGeometry(this->rect());
    turnoLabel->setGeometry(overlay->rect());

    for (int i = 0; i < posiciones.size(); ++i) {
        posiciones[i]->mostrarPosicion();
    }

}

// FUNCIONES PARA COMUNICARSE CON BACKEND Y JUGAR PARTIDAS

// Función auxiliar para crear un diálogo modal con mensaje personalizado.
// Si exitApp es verdadero, al cerrar se finaliza la aplicación.
static QDialog* createDialog(QWidget *parent, const QString &message, bool exitApp) {
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
QString GameWindow::loadAuthToken(const QString &userKey) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
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
            playerPosMap[id] = pos;
            playerPosMap[this->player_id] = 0;
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
        int jugadorId = data["jugador"].toObject()["id"].toInt();
        QString nombre = data["jugador"].toObject()["nombre"].toString();

        if (jugadorId == player_id) {
            posiciones[0]->setLock(false);
            mostrarTurno("Es tu turno", true);
        } else {
            posiciones[0]->setLock(true);
            mostrarTurno("Es el turno de " + nombre, false);
        }
    }


    else if (type == "start_game") {
        if(data.contains("chat_id")) {
            chatID = QString::number(data["chat_id"].toInt());
        }
        // inicializo nombres y marcador
        QJsonArray jugadores = data["jugadores"].toArray();
        for (auto v : jugadores) {
            auto obj = v.toObject();
            int   id   = obj["id"].toInt();
            auto  name = obj["nombre"].toString();
        }

    }

    else if (type == "card_played") {
        int jugadorId = data["jugador"].toObject()["id"].toInt();
        int idx       = playerPosMap.value(jugadorId, -1);
        if (idx < 0) return;

        if (jugadorId == player_id) {
            return;
        }

        // --- Sólo oponente a partir de aquí ---

        Carta* backCarta = manos[idx]->cartas.first();
        QPoint start = backCarta->mapTo(this, QPoint(0,0));

        manos[idx]->eliminarCarta(0);

        QString palo = data["carta"].toObject()["palo"].toString();
        int valor    = data["carta"].toObject()["valor"].toInt();
        // 1) Creamos la carta ya en modo “cara arriba”
        Carta* carta = new Carta(this, this,
                                 QString::number(valor),
                                 palo,
                                 cardSize,
                                 /*skin=*/0,
                                 /*faceUp=*/true);
        addCartaPorId(carta);
        carta->move(start);
        carta->show();
        carta->raise();


         // 2) Calculamos el destino: centro de la Posicion
         Posicion* posWidget = posiciones[idx];
         QPoint dst = posWidget->mapTo(this,
                               QPoint((posWidget->width()  - carta->width())/2,
                                      (posWidget->height() - carta->height())/2));

         // 3) Flip‐anim: rotación Y de 0→180º
         {
             auto *flip = new QPropertyAnimation(carta, "rotationY", this);
             flip->setDuration(500);
             flip->setStartValue(0.0);
             flip->setEndValue(180.0);
             flip->setEasingCurve(QEasingCurve::InOutQuad);
             flip->start(QAbstractAnimation::DeleteWhenStopped);
         }

         // 4) Animamos simultáneamente el movimiento
         {
             auto *moveAnim = new QPropertyAnimation(carta, "pos", this);
             moveAnim->setDuration(500);
             moveAnim->setStartValue(start);
             moveAnim->setEndValue(dst);
             connect(moveAnim, &QPropertyAnimation::finished, [=]() {
                 posWidget->setCard(carta);
                 backCarta->deleteLater();
             });
             moveAnim->start(QAbstractAnimation::DeleteWhenStopped);
         }
        // 6) Desbloqueamos siguiente
        int siguiente = (idx + 1) % posiciones.size();
        posiciones[siguiente]->setLock(false);
    }


    else if (type == "round_result") {
        // 1) Info del ganador
        int ganadorId      = data["ganador"].toObject()["id"].toInt();
        QString ganadorNom = data["ganador"].toObject()["nombre"].toString();

        // 2) Overlay anunciador
        mostrarTurno(QString("¡Gana la baza: %1!").arg(ganadorNom),
                     ganadorId == player_id);

        // 3) Extraer **tanto** cartaActual **como** cualquier hija directa
        struct Slot { Posicion* posW; Carta* c; QPoint start; };
        QVector<Slot> slotList;
        for (Posicion* posW : posiciones) {
            // 3a) Si hay cartaActual, la cogemos
            if (Carta* c = posW->cartaActual) {
                QPoint g = c->mapToGlobal(QPoint(0,0));
                QPoint start = mapFromGlobal(g);
                slotList.append({ posW, c, start });
                posW->cartaActual = nullptr;
                c->setParent(this);
                c->move(start);
                c->show(); c->raise();
            }
            // 3b) Y buscamos cualquier otra Carta hija directa
            auto hijas = posW->findChildren<Carta*>(QString(), Qt::FindDirectChildrenOnly);
            for (Carta* c : hijas) {
                // si ya era la cartaActual, la ignoramos
                // 3b) Y buscamos cualquier otra Carta hija directa
                auto hijas = posW->findChildren<Carta*>(QString(), Qt::FindDirectChildrenOnly);
                for (Carta* c : hijas) {
                    // comprobamos si ya la añadimos
                    bool already = false;
                    for (const Slot &s : slotList) {
                        if (s.c == c) { already = true; break; }
                    }
                    if (already)
                        continue;

                    QPoint g = c->mapToGlobal(QPoint(0,0));
                    QPoint start = mapFromGlobal(g);
                    slotList.append({ posW, c, start });
                    c->setParent(this);
                    c->move(start);
                    c->show(); c->raise();
                }

                QPoint g = c->mapToGlobal(QPoint(0,0));
                QPoint start = mapFromGlobal(g);
                slotList.append({ posW, c, start });
                c->setParent(this);
                c->move(start);
                c->show(); c->raise();
            }
        }

        // 4) Desbloquear
        for (Posicion* posW : posiciones)
            posW->setLock(false);

        // 5) Animar hacia la esquina sólo sota/rey, el resto se destruye
        const bool userWon = (ganadorId == player_id);
        QPoint base = userWon
                          ? QPoint(width()  - winPileMargin - cardSize,
                                   height() - winPileMargin - cardSize)
                          : QPoint(winPileMargin, winPileMargin);
        int &count = userWon ? winPileCountUser : winPileCountOpponent;

        for (const Slot &sl : slotList) {
            Carta* c = sl.c;
            c->hideFace();
            bool keep = (c->num == "10" || c->num == "12");
            QPoint dst = base + QPoint(count * winPileOffset,
                                       count * winPileOffset);
            if (keep) ++count;
            auto *anim = new QPropertyAnimation(c, "pos", this);
            anim->setDuration(500);
            anim->setStartValue(sl.start);
            anim->setEndValue(dst);
            connect(anim, &QPropertyAnimation::finished, [c, keep]() {
                if (!keep) c->deleteLater();
            });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }







    else if (type == "phase_update") {
        arrastre = true;
        mostrarTurno(data["message"].toString(), false);
    }

    else if (type == "card_drawn") {
        QString palo = data["carta"].toObject()["palo"].toString();
        int valor    = data["carta"].toObject()["valor"].toInt();

        if (data["usuario"].toObject().value("id").toInt() == player_id) {
            // me toca a mí
            Carta* carta = new Carta(this, this, QString::number(valor), palo, cardSize, 0);
            addCartaPorId(carta);
            manos[0]->añadirCarta(carta);
        } else {
            // oponente, sólo añadimos reverso
            int idx = playerPosMap.value(
                data["usuario"].toObject().value("id").toInt(), -1);
            if (idx >= 0) {
                Carta* back = new Carta(this, this, "0", "", cardSize, 0);
                manos[idx]->añadirCarta(back);
            }
        }
        // Actualizar mazo
        deck->cartaRobada();
    }

    else if (type == "player_left") {
        QString msg = data["message"].toString();
        QString nombre = data["usuario"].toObject()["nombre"].toString();
        qDebug() << nombre << "se desconectó." << msg;
    }
    else if (type == "end_game") {
        QString msg    = data["message"].toString();
        int ganadorEq  = data["ganador_equipo"].toInt();
        int p1 = data["puntos_equipo_1"].toInt();
        int p2 = data["puntos_equipo_2"].toInt();

        QDialog *d = createDialog(this,
                                  QString("%1\n\nEquipo %2 gana!\nPuntos: E1=%3  E2=%4")
                                      .arg(msg).arg(ganadorEq).arg(p1).arg(p2),
                                  false);
        d->show();
    }

    else {
        qDebug() << "Mensaje desconocido:" << mensaje;
    }
}
