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
#include <QSettings>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QTimer>
#include <QtWebSockets>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QPoint>//


QMap<QString, Carta*> GameWindow::cartasPorId;
static QDialog* createDialog(QWidget *parent, const QString &message, bool exitApp = false);

/* Devuelve un nombre “corto” para cada skin.
 * Amplíalo cada vez que añadas una baraja nueva.
 */
static QString skinName(int idx)
{
    switch (idx) {
    case 0:  return "base";   // reverso / baraja clásica
    case 1:  return "poker";
    default: return QString("skin%1").arg(idx);
    }
}

void GameWindow::addCartaPorId(Carta* c){
    cartasPorId[c->idGlobal] = c;
    // Cuando Qt destruya la Carta, la quitamos del mapa automáticamente
    connect(c, &QObject::destroyed, this, [id = c->idGlobal]() {
        GameWindow::cartasPorId.remove(id);
    });
}


Carta* GameWindow::getCartaPorId(QString id){
    return cartasPorId.value(id, nullptr);
}

GameWindow::GameWindow(const QString &userKey, int type, int fondo, QJsonObject msg, int id, QWebSocket *ws, QString usr, MenuWindow *menuRef)
    : legendLabel(nullptr), legendPinned(false) {
    QString config = QString("Sota, Caballo y Rey_%1").arg(usr);
    QSettings appSettings("Grace Hopper", config);
    this->deckSkin = appSettings.value("selectedDeck", 0).toInt();


    this->usr = usr;
    menuWindowRef = menuRef;
    bg = fondo;
    gameType = type;
    player_id=id;
    cardSize = 175;
    this->ws = ws;
    hasPendingDraw     = false;
    pendingDrawUserId  = -1;
    pendingDrawData    = QJsonObject();
    sfxPlayer = new QMediaPlayer(this);
    sfxOutput = new QAudioOutput(this);
    int effectVolume = appSettings.value("sound/effectsVolume", 50).toInt();
    sfxOutput->setVolume(static_cast<double>(effectVolume) / 100.0);
    sfxPlayer->setAudioOutput(sfxOutput);

    QObject::connect(ws, &QWebSocket::textMessageReceived,
                     this, &GameWindow::recibirMensajes,
                     Qt::UniqueConnection);
    token = loadAuthToken(userKey);
    setBackground();
    if (msg.contains("chat_id")) {
        chatID = QString::number(msg["chat_id"].toInt());
    }
    setupUI(userKey);
    int volume = appSettings.value("sound/volume", 50).toInt();

    // Crear reproductor de música
    backgroundPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    backgroundPlayer->setAudioOutput(audioOutput);

    // Establecer volumen y archivo
    audioOutput->setVolume(static_cast<double>(volume) / 100.0);
    backgroundPlayer->setSource(QUrl("qrc:/bgm/partida.mp3"));
    backgroundPlayer->setLoops(QMediaPlayer::Infinite);
    backgroundPlayer->play();
    setupGameElements(msg);
    setMouseTracking(true);
    optionsBar->setMouseTracking(true);

    if (msg.contains("gameID") && msg["gameID"].isString()) {
        gameID = msg["gameID"].toString();
    }

}

void GameWindow::colocarLeyenda()
{
    if (legendPinned || !legendLabel)          // ya está, o aún no existe
        return;

    if (deckSkin == 0)
        return;                               // baraja “base” → sin leyenda

    QString ruta = QString(":/legends/legend%1.png")
                       .arg(skinName(deckSkin));

    QPixmap pm(ruta);
    if (pm.isNull()) {
        qWarning() << "[GameWindow] Leyenda no encontrada:" << ruta;
        return;
    }
    legendLabel->setPixmap(pm.scaled(200,200,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
    legendLabel->show();
    legendPinned = true;
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

    // ------------------------------------------------------------------
    // Leyenda (se crea una sola vez)
    // ------------------------------------------------------------------
    if (!legendLabel) {                       // evita recrearla
        legendLabel = new QLabel(this);
        legendLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

        legendLabel->setAttribute(Qt::WA_TranslucentBackground);
        legendLabel->setStyleSheet("background: transparent;");

        legendLabel->move(15, 360);            // esquina superior-izq.
    }
    legendLabel->hide();                      // por defecto oculta
    // NUEVO:
    colocarLeyenda();              // ya existe la label: decide y muestra

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
    settings->setImage(":/icons/audio.png", 50, 50);
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
        settings->setImage(":/icons/darkenedaudio.png", 50, 50);
        SettingsWindow *w = new SettingsWindow(menuWindowRef, this, usr);
        w->setModal(true);
        connect(w, &QDialog::finished, [=](int){ settings->setImage(":/icons/audio.png",50,50); });
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
    hideTurnoTimer = new QTimer(this);
    hideTurnoTimer->setSingleShot(true);
    connect(hideTurnoTimer, &QTimer::timeout,
            this, &GameWindow::ocultarTurno);

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

    // Calculamos la duración (2500ms + 500ms extra en caso de “ganado la mano”)
    int duracion = 2500;
    if (texto.contains("ganado la mano", Qt::CaseInsensitive))
        duracion += 500;
    // Reiniciamos el único timer: si vuelve a llegar otro mensaje antes de expirar,
    // cancelamos el ocultado y lo programamos al final del nuevo mensaje.
    hideTurnoTimer->start(duracion);
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
    winPileCounts.clear();
    winPileCounts.resize(posiciones.size());
    winPileCounts.fill(0);

    pileBacks.clear();
    for (int i = 0; i < posiciones.size(); ++i)
        pileBacks[i] = {};

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

    if (legendLabel)
        legendLabel->raise();

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
    qDebug() << "🧪 deckSkin en setupGameState =" << deckSkin;
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
        Carta* carta = new Carta(this, this, val, palo, cardSize, deckSkin);

        // Y agregarla a la mano, por ejemplo:
        manos[0]->añadirCarta(carta);
    }

    // Inicializamos Resto de Jugadores
    QJsonArray jugadoresArray = s0.value("jugadores").toArray();

    int i = 0;
    int posOrder[] = {2, 1, 3};
    int pos;

    for (const QJsonValue& jugadorVal : jugadoresArray) {
        QJsonObject jugadorObj = jugadorVal.toObject();

        int id = jugadorObj.value("id").toInt();
        QString nombre = jugadorObj.value("nombre").toString();
        int equipo = jugadorObj.value("equipo").toInt();
        int numCartas = jugadorObj.value("num_cartas").toInt();
        if (id != player_id) {

            if(gameType == 2){
                pos = posOrder[i];
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
    qDebug() << "[recibirMensajes] ⟶ WS raw message:" << mensaje;

    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();
    qDebug() << "[recibirMensajes] ⟶ type =" << type;

    QJsonObject data = obj.value("data").toObject();

    if (type == "turn_update") {
        // Si aún estamos animando el resultado de la ronda, lo guardamos
        if (roundResultInProgress) {
            pendingTurnUpdateData = obj["data"].toObject();
            return;
        }
        // Si no, lo procesamos inmediatamente
        processTurnUpdate(obj["data"].toObject());
    }


    else if (type == "start_game") {
        qDebug() << "   ▶ start_game: data =" << data;

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
        int jugadorId    = data["jugador"].toObject()["id"].toInt();
        bool esAutomatica = data["automatica"].toBool(false);
        int idx          = playerPosMap.value(jugadorId, -1);
        if (idx < 0) return;

        if (jugadorId == player_id && !esAutomatica) {
            // Como ya animaste la carta al hacer clic, aquí sólo la quitamos de la mano
            QJsonObject cartaJson = data["carta"].toObject();
            QString paloServ = cartaJson["palo"].toString();
            QString numServ  = QString::number(cartaJson["valor"].toInt());
            Mano* miMano = manos[0];
            // Busca y elimina esa carta de mi mano
            for (int i = 0; i < miMano->cartas.size(); ++i) {
                Carta* c = miMano->cartas[i];
                if (c->num == numServ && c->suit == paloServ) {
                    miMano->eliminarCarta(i);
                    break;
                }
            }
            // No hacemos más animaciones aquí
            return;
        }

        // --- Aquí procesamos tanto las jugadas del oponente como las automáticas propias ---
        QPoint start;
        Carta* cartaASacar = nullptr;

        if (jugadorId == player_id) {
            // 1) Extrae palo/valor del JSON
            QJsonObject cartaJson = data["carta"].toObject();
            QString paloServ = cartaJson["palo"].toString();
            QString numServ  = QString::number(cartaJson["valor"].toInt());

            // 2) Busca en mi mano la carta con ese palo+valor
            Mano* miMano = manos[0];
            int foundIdx = -1;
            for (int i = 0; i < miMano->cartas.size(); ++i) {
                Carta* c = miMano->cartas[i];
                if (c->num == numServ && c->suit == paloServ) {
                    foundIdx = i;
                    break;
                }
            }
            // si no la encuentra (por seguridad), usa la primera
            if (foundIdx < 0) foundIdx = 0;

            // 3) Ahora extrae esa carta concreta
            Carta* cartaASacar = miMano->cartas[foundIdx];
            start = cartaASacar->mapTo(this, QPoint(0,0));

            // 4) La eliminas de la mano lógica y la refrescas
            miMano->eliminarCarta(foundIdx);
            miMano->mostrarMano();
            qApp->processEvents();

            // 5) Sigue con el mismo código de animación:
            cartaASacar->setParent(this);
            cartaASacar->move(start);
            cartaASacar->show();
            cartaASacar->raise();

            Posicion* posWidget = posiciones[0];
            QPoint dst = posWidget->mapTo(this,
                                          QPoint((posWidget->width() - cartaASacar->width())/2,
                                                 (posWidget->height() - cartaASacar->height())/2));



            auto *moveAnim = new QPropertyAnimation(cartaASacar, "pos", this);
            moveAnim->setDuration(500);
            moveAnim->setStartValue(start);
            moveAnim->setEndValue(dst);
            connect(moveAnim, &QPropertyAnimation::finished, [=]() {
                posWidget->setCard(cartaASacar);
            });
            moveAnim->start(QAbstractAnimation::DeleteWhenStopped);

            // desbloquea el siguiente turno…
            posiciones[1 % posiciones.size()]->setLock(false);
        }  else {
        // 1) Nos quedamos con el puntero al dorso que vamos a animar
        Carta* backCarta = manos[idx]->cartas.first();
        // 1b) Capturamos la posición de ese dorso (antes de eliminarlo)
        start = backCarta->mapTo(this, QPoint(0,0));

        // 2) Lo eliminamos de la mano lógica
        manos[idx]->eliminarCarta(0);
        // 3) También lo eliminamos del widget (evita el “fantasma”)
        backCarta->deleteLater();

        // 4) Actualizamos visualmente la mano
        manos[idx]->mostrarMano();
        qApp->processEvents();

        // 5) Creamos la carta cara‐arriba y la añadimos
        int valor     = data["carta"].toObject()["valor"].toInt();
        QString palo  = data["carta"].toObject()["palo"].toString();
        int skin = (jugadorId == player_id) ? deckSkin : 0;
        cartaASacar = new Carta(this, this,
                                QString::number(valor),
                                palo,
                                cardSize,
                                skin,
                                /*faceUp=*/true);
        addCartaPorId(cartaASacar);
    }


        // A partir de aquí, animamos 'cartaASacar' desde start hasta el centro de la posición:
        cartaASacar->move(start);
        cartaASacar->show();
        cartaASacar->raise();

        Posicion* posWidget = posiciones[idx];
        QPoint dst = posWidget->mapTo(this,
                                      QPoint((posWidget->width()  - cartaASacar->width())/2,
                                             (posWidget->height() - cartaASacar->height())/2));



        // Movimiento
        auto *moveAnim = new QPropertyAnimation(cartaASacar, "pos", this);
        moveAnim->setDuration(500);
        moveAnim->setStartValue(start);
        moveAnim->setEndValue(dst);
        connect(moveAnim, &QPropertyAnimation::finished, [=]() {
            posWidget->setCard(cartaASacar);
            // si era back de oponente, borramos el back original
            if (jugadorId != player_id) {
                // backCarta ya fue eliminado arriba
            }
        });
        moveAnim->start(QAbstractAnimation::DeleteWhenStopped);

        // Finalmente desbloqueamos el siguiente turno
        int siguiente = (idx + 1) % posiciones.size();
        posiciones[siguiente]->setLock(false);
    }



    else if (type == "round_result") {
        qDebug() << "[recibirMensajes] ▶ Entrando al handler de round_result";
        qDebug() << "[recibirMensajes]    Datos:" << data;

        // 1) Si ya estoy procesando una mano, guardo y salgo:
        if (roundResultInProgress) {
            qDebug() << "[round_result] 🔜 Animación en curso, guardando pendingRoundResultData";
            pendingRoundResultData = data;
            return;
        }

        // 2) Marco que inicio la animación
        roundResultInProgress = true;
        qDebug() << "[round_result] roundResultInProgress = true";

        // 3) Programo la ejecución diferida
        qDebug() << "[round_result] Programando procesarRoundResultSeguro en 550 ms";
        QTimer::singleShot(550, this, [this, data]() {
            procesarRoundResultSeguro(data);
        });

        qDebug() << "[recibirMensajes] ✔ terminado setup de round_result";
    }









    else if (type == "phase_update") {
        qDebug() << "   ▶ phase_update: mensaje =" << data["message"].toString();

        arrastre = true;
        mostrarTurno(data["message"].toString(), false);
    }

    else if (type == "card_drawn") {
        // Extraer ID de quién roba
        int userId = player_id;
        if (data.contains("usuario"))
            userId = data["usuario"].toObject().value("id").toInt();
        else if (data.contains("jugador"))
            userId = data["jugador"].toObject().value("id").toInt();

        // Lista de todos a animar
        QVector<int> drawIds;
        drawIds.append(userId);
        // Si quieres también animar a los demás:
        for (auto it = playerPosMap.constBegin(); it != playerPosMap.constEnd(); ++it)
            if (it.key() != userId)
                drawIds.append(it.key());

        if (roundResultInProgress) {
            // guardamos TODO el paquete + IDs pendientes
            pendingDrawData    = data;
            pendingDrawUserIds = drawIds;
            hasPendingDraw     = true;
        } else {
            // animamos ahora mismo para cada uno
            for (int id : drawIds) {
                QTimer::singleShot(800, this, [=]() {
                    animateDraw(data, id);
                });
            }
        }
        return;
    }





    else if (type == "player_left") {
        QString msg = data["message"].toString();
        QString nombre = data["usuario"].toObject()["nombre"].toString();
        qDebug() << nombre << "se desconectó." << msg;
    }
    else if (type == "end_game") {
        qDebug() << "   ▶ end_game: data =" << data;

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


void GameWindow::animateDraw(const QJsonObject &drawData, int userId) {
    // ▶ Retrasar animación de robo hasta que termine el movimiento de la pila
    QPoint start = deck->mapTo(this,
                               QPoint((deck->width()  - cardSize) / 2,
                                      (deck->height() - cardSize) / 2));

    // Determinar la mano destino
    Mano* mano = manos[0];
    if (userId != player_id) {
        int idx = playerPosMap.value(userId, -1);
        if (idx < 0) return;
        mano = manos[idx];
    }

    // Extraer datos de carta (pero para oponente sólo usaremos dorso)
    int valor = drawData["carta"].toObject()["valor"].toInt();
    QString palo = drawData["carta"].toObject()["palo"].toString();

    bool faceUp = (userId == player_id);
    QString num = faceUp ? QString::number(valor) : QString("0");
    QString paloParaCarta = faceUp ? palo : "";
    int skinToUse = faceUp ? deckSkin : 0;
    // 1) Crear carta pero NO mostrarla aún
    Carta* carta = new Carta(this, this,
                             num,
                             paloParaCarta,
                             cardSize,
                             skinToUse,
                             /*faceUp=*/faceUp);
    if (faceUp) addCartaPorId(carta);

    // 2) La agregamos a la mano **y** hacemos layout **antes** de animar:
    mano->añadirCarta(carta);
    mano->mostrarMano();

    // 3) Capturamos la posición definitiva tras el layout:
    QPoint finalPos = carta->pos();

    // 4) La colocamos de nuevo en el mazo (start) y la mostramos
    carta->move(start);
    carta->show();
    carta->raise();

    // 🔊 EFECTO DE SONIDO AQUÍ
    sfxPlayer->setSource(QUrl("qrc:/bgm/card_draw.mp3"));
    sfxPlayer->play();

    // 5) Animamos de start → finalPos
    auto *anim = new QPropertyAnimation(carta, "pos", this);
    anim->setDuration(500);
    anim->setStartValue(start);
    anim->setEndValue(finalPos);
    anim->setEasingCurve(QEasingCurve::OutQuad);

    // 6) Al acabar, volvemos a layout para asegurar simetría total
    connect(anim, &QPropertyAnimation::finished, [this, mano]() {
        mano->mostrarMano();
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 7) Actualizamos visual de mazo
    deck->cartaRobada();
    deck->actualizarVisual();
}

void GameWindow::processTurnUpdate(const QJsonObject &data) {
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

GameWindow::~GameWindow() {

    // 1) Parar y desconectar todo
    hideTurnoTimer->stop();
    hideOptionsTimer->stop();
    if (fadeIn)   { fadeIn  ->stop(); fadeIn->deleteLater();   fadeIn  = nullptr; }
    if (fadeOut)  { fadeOut ->stop(); fadeOut->deleteLater();  fadeOut = nullptr; }
    if (currentRoundAnim) {
        currentRoundAnim->stop();
        currentRoundAnim->deleteLater();
        currentRoundAnim = nullptr;
    }
    // 2) Desconectar websockets y señales locales
    ws->disconnect(this);
    this->disconnect();
    // 3) Limpiar cartas estáticas
    cartasPorId.clear();

    if (backgroundPlayer) backgroundPlayer->stop();

}

void GameWindow::procesarRoundResultSeguro(const QJsonObject& data) {
    qDebug() << "[procesarRoundResultSeguro] — entrada";

    // 1) Info del ganador
    int ganadorId      = data["ganador"].toObject()["id"].toInt();
    QString ganadorNom = data["ganador"].toObject()["nombre"].toString();

    qDebug() << ">>> round_result recibido. ganadorId =" << ganadorId
             << ", nombre =" << ganadorNom;
    // 2) Overlay anunciador
    if (ganadorId == player_id) {
        // Mensaje para quien gana
        mostrarTurno(QString("¡Has ganado la mano!"), true);
    } else {
        // Mensaje para quien pierde
        mostrarTurno(QString("¡%1 ha ganado la mano!").arg(ganadorNom), false);
    }

    // 3) Extraer **tanto** cartaActual **como** cualquier hija directa
    struct Slot { Posicion* posW; Carta* c; QPoint start; };
    QVector<Slot> slotList;
    QSet<Carta*> yaProcesadas;
    for (int i = 0; i < posiciones.size(); ++i) {
        Posicion* posW = posiciones[i];
        qDebug() << "[RoundResult] Revisión de posición" << i << "con posW =" << posW;

        // cartaActual
        if (Carta* c = posW->cartaActual) {
            qDebug() << "  ↪ cartaActual detectada: num=" << c->num << "suit=" << c->suit << "parent=" << c->parent();
            if (!c->isVisible() || c->parent() == nullptr) {
                qWarning() << "  ⚠️ cartaActual INVÁLIDA — Se omite";
                continue;
            }

            QPoint g = c->mapToGlobal(QPoint(0,0));
            QPoint start = mapFromGlobal(g);
            slotList.append({ posW, c, start });
            posW->cartaActual = nullptr;
            c->setParent(this);
            c->move(start); c->show(); c->raise();
            yaProcesadas.insert(c);
            qDebug() << "  ✔ cartaActual agregada a slotList";
        } else {
            qDebug() << "  ↪ Sin cartaActual en posición" << i;
        }

        // Hijas directas
        const auto hijas = posW->findChildren<Carta*>(QString(), Qt::FindDirectChildrenOnly);
        qDebug() << "  ↪ Hijas encontradas:" << hijas.size();
        for (Carta* c : hijas) {
            if (!c) {
                qWarning() << "  ⚠️ hija nula";
                continue;
            }
            if (yaProcesadas.contains(c)) {
                qDebug() << "  ⤷ Saltada (ya procesada)";
                continue;
            }
            if (c->parent() == nullptr || !c->isVisible()) {
                qWarning() << "  ⚠️ hija INVÁLIDA — parent=" << c->parent() << "visible=" << c->isVisible();
                continue;
            }

            bool alreadyInList = std::any_of(slotList.begin(), slotList.end(), [c](const Slot &s) {
                return s.c == c;
            });
            if (alreadyInList) {
                qWarning() << "  ⚠️ Duplicado en slotList, se omite";
                continue;
            }

            QPoint g = c->mapToGlobal(QPoint(0,0));
            QPoint start = mapFromGlobal(g);

            slotList.append({ posW, c, start });
            c->setParent(this);
            c->move(start); c->show(); c->raise();
            yaProcesadas.insert(c);
            qDebug() << "  ✔ hija agregada a slotList: num=" << c->num << "suit=" << c->suit;
        }

        posW->update();
    }


    // 4) Desbloquear
    for (Posicion* posW : posiciones)
        posW->setLock(false);
    // 5) Actualizamos posiciones
    int ganadorPos = playerPosMap.value(ganadorId, 0);
    QPoint base;

    // detectamos 2-jugadores por posiciones.size()
    if (posiciones.size() == 2) {
        // 2 players: 0 = yo abajo-derecha, 1 = rival arriba-izquierda
        if (ganadorPos == 0) {
            base = QPoint(
                width()  - winPileMargin - cardSize,
                height() - winPileMargin - cardSize
                );
        } else {
            base = QPoint(
                winPileMargin,
                winPileMargin
                );
        }
    }
    else {
        // 4 players: 0=abajo-derecha, 1=arriba-centro, 2=izquierda, 3=derecha
        switch (ganadorPos) {
        case 0:
            base = QPoint(
                width()  - winPileMargin - cardSize,
                height() - winPileMargin - cardSize
                );
            break;
        case 1:
            base = QPoint(
                (width()  - cardSize) / 2,
                winPileMargin
                );
            break;
        case 2:
            base = QPoint(
                winPileMargin,
                (height() - cardSize) / 2
                );
            break;
        case 3:
            base = QPoint(
                width()  - winPileMargin - cardSize,
                (height() - cardSize) / 2
                );
            break;
        default:
            base = QPoint(winPileMargin, winPileMargin);
        }
    }
    qDebug() << "→ posiciones.size() =" << posiciones.size()
             << "ganadorPos =" << ganadorPos
             << "→ base =" << base;
    int &count = winPileCounts[ganadorPos];  // Usa un array/vector de contadores por posición
    qDebug() << "Contador previo (winPileCounts[" << ganadorPos << "]) =" << count;


    // Creamos una animación secuencial de 3 fases y la guardamos
    qDebug() << "[procesarRoundResultSeguro] — antes de crear QSequentialAnimationGroup";

    currentRoundAnim = new QSequentialAnimationGroup(this);

    // FASE 2: volver al centro
    auto *ret = new QParallelAnimationGroup(currentRoundAnim);
    qDebug() << "→ SlotList contiene" << slotList.size() << "elementos:";
    for (const Slot &sl : slotList) {
        qDebug() << "   ↪ Carta:" << sl.c << "num=" << sl.c->num << "suit=" << sl.c->suit
                 << "parent=" << sl.c->parent() << "pos=" << sl.c->pos();
    }

    for (const Slot &sl : slotList) {
        Carta* c = sl.c;
        auto *a = new QPropertyAnimation(c, "pos");
        a->setDuration(300);
        a->setStartValue(c->pos());
        a->setEndValue(sl.start);
        a->setEasingCurve(QEasingCurve::InOutQuad);
        ret->addAnimation(a);
    }
    currentRoundAnim->addAnimation(ret);

    // PAUSA antes de mostrar mensaje
    auto *pause = new QPauseAnimation(200, currentRoundAnim);
    currentRoundAnim->addAnimation(pause);
    connect(pause, &QPauseAnimation::finished, this, [=](){
        if (ganadorId == player_id)
            mostrarTurno("¡Has ganado la mano!", true);
        else
            mostrarTurno(QString("¡%1 ha ganado la mano!").arg(ganadorNom), false);
    });

    // FASE 3: mover a la pila de triunfo
    auto *move = new QParallelAnimationGroup(currentRoundAnim);
    qDebug() << "→ SlotList contiene" << slotList.size() << "elementos:";
    for (const Slot &sl : slotList) {
        qDebug() << "   ↪ Carta:" << sl.c << "num=" << sl.c->num << "suit=" << sl.c->suit
                 << "parent=" << sl.c->parent() << "pos=" << sl.c->pos();
    }

    for (const Slot &sl : slotList) {
        Carta* c = sl.c;
        bool keep = (c->num == "10" || c->num == "12");
        QPoint dst = base + QPoint(count * winPileOffset,
                                   count * winPileOffset);
        if (keep) ++count;
        if (c->parent() == nullptr || c->isHidden()) {
            qWarning() << "⚠️ Se intentó reanimar una carta ya destruida.";
            continue;
        }
        auto *a = new QPropertyAnimation(c, "pos");
        a->setDuration(500);
        a->setStartValue(c->pos());
        a->setEndValue(dst);
        a->setEasingCurve(QEasingCurve::InOutQuad);
        move->addAnimation(a);
        connect(a, &QPropertyAnimation::finished, [c, keep](){

            if (!keep) c->deleteLater();
        });
    }
    currentRoundAnim->addAnimation(move);

    // Al acabar, regenerar los reversos y refrescar manos
    connect(move, &QParallelAnimationGroup::finished, this, [this, ganadorPos, base]() {
        qDebug() << "[move.finished] — start limpieza de pileBacks para ganadorPos =" << ganadorPos;

        for (Carta* oldBack : pileBacks[ganadorPos]) {
            qDebug() << "   -> deleteLater de oldBack" << oldBack;
            oldBack->deleteLater();
        }

        pileBacks[ganadorPos].clear();
        for (int i = 0; i < 2; ++i) {
            Carta* back = new Carta(this, this, "0", "", cardSize, 0, false);
            QPoint p = base + QPoint(i * pileBackOffset, i * pileBackOffset);
            back->move(p); back->show();
            pileBacks[ganadorPos].append(back);
        }
        for (Mano* m : manos) m->mostrarMano();
        repositionHands();
        update();
        // Acabamos de procesar el resultado de la ronda: liberamos la marca
        qDebug() << "[move.finished] — roundResultInProgress = false";
        roundResultInProgress = false;

        // Si había un round_result pendiente, lo lanzo ahora

        if (!pendingRoundResultData.isEmpty()) {
            QJsonObject next = std::move(pendingRoundResultData);
            pendingRoundResultData = QJsonObject();
            qDebug() << "[move.finished] 🔔 Llamando directo a procesarRoundResultSeguro";
            QTimer::singleShot(300, this, [this, next]() {
                procesarRoundResultSeguro(next);
            });
        }
        // Si había un turn_update pendiente, lo procesamos ahora
        if (!pendingTurnUpdateData.isEmpty()) {
            QTimer::singleShot(1000, this, [this, data = pendingTurnUpdateData]() {
                processTurnUpdate(data);
            });
            pendingTurnUpdateData = QJsonObject();
        }
    });

    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        if (!hasPendingDraw) return;
        QTimer::singleShot(500, this, [this]() {
            for (int id : pendingDrawUserIds)
                animateDraw(pendingDrawData, id);
            hasPendingDraw = false;
            pendingDrawUserIds.clear();
        });
    });


    connect(currentRoundAnim, &QSequentialAnimationGroup::finished, this, [this]() {
        qDebug() << "[currentRoundAnim.finished] eliminando y limpiando puntero";
        currentRoundAnim->deleteLater();
        currentRoundAnim = nullptr;
        qDebug() << "[currentRoundAnim.finished] — puntero a nullptr, listo";
        qDebug() << "[currentRoundAnim.finished] — justo antes de retornar al event loop";

    });

    qDebug() << "[procesarRoundResultSeguro] — antes de start()";

    currentRoundAnim->start();

    qDebug() << "[procesarRoundResultSeguro] — salida (animaciones en marcha)";


    pendingRoundResult = nullptr;

    for (Mano* mano : manos) {
        mano->mostrarMano();  // Forzar actualización de todas las manos
    }
    repositionHands();       // Sincronizar posiciones globales
    update();                // Redibujar toda la ventana

    repositionHands();
};

void GameWindow::getSettings() {
    QString config = "Sota, Caballo y Rey_" + usr;
    QSettings settings("Grace Hopper", config);
    int volume = settings.value("sound/volume", 50).toInt();
    qDebug() << "Cargando configuración - Volumen:" << volume;

    // Lo quito para depurar TODO: this->showFullScreen();

    MenuWindow *menuWin = qobject_cast<MenuWindow*>(menuWindowRef);
    if (menuWin && menuWin->audioOutput) {
        menuWin->audioOutput->setVolume(static_cast<double>(volume) / 100.0);
    } else {
        qWarning() << "Error: no se pudo acceder a audioOutput desde MenuWindow.";
    }
    this->update();
}
