/**
 * @file EstadoPartida.cpp
 * @brief Implementación de la clase EstadoPartida, gestiona la lógica y presentación de la partida de cartas.
 */

#include "estadopartida.h"
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QSettings>
#include <QJsonArray>
#include <QScreen>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPushButton>
#include <QDialog>
#include <QJsonDocument>
#include "ventanasalirpartida.h"

/**
 * @brief Constructor de EstadoPartida.
 *
 * Inicializa el estilo de la ventana y el skin de las cartas.
 *
 * @param miNombre Nombre del jugador local.
 * @param wsUrl URL del servidor WebSocket.
 * @param bg Selección de fondo (0: verde claro, distinto: rojo oscuro).
 * @param style Skin a usar en las cartas.
 * @param onSalir Callback que se llamará al salir de la partida.
 * @param parent Widget padre (opcional).
 */
EstadoPartida::EstadoPartida(QString miNombre, const QString& wsUrl, int bg, int style,
                             std::function<void()> onSalir, QWidget* parent)
    : QWidget(parent), miNombre(miNombre), onSalir(onSalir), wsUrl(wsUrl) {
    if(bg == 0) {
        this->setStyleSheet(R"(
            /* Fondo de la ventana con gradiente verde */
            QWidget {
                background: qradialgradient(
                    cx:0.5, cy:0.5, radius:1,
                    fx:0.5, fy:0.5,
                    stop:0 #1f5a1f,
                    stop:1 #0a2a08
                );
            }
        )");
    } else {
        this->setStyleSheet(R"(
            /* Fondo de la ventana con gradiente verde */
            QWidget {
                background: qradialgradient(
                    cx:0.5, cy:0.5, radius:1,
                    fx:0.5, fy:0.5,
                    stop:0 #5a1f1f,
                    stop:1 #2a0808
                );
            }
        )");
    }
    Carta::skin = style;

    //
    // ——— BGM DE PARTIDA ———
    //
    audioOutput      = new QAudioOutput(this);
    backgroundPlayer = new QMediaPlayer(this);
    backgroundPlayer->setAudioOutput(audioOutput);

    // Leemos ambos volúmenes de la misma QSettings
    QSettings cfg("Grace Hopper",
                  QString("Sota, Caballo y Rey_%1").arg(miNombre));
    int bgmVol    = cfg.value("sound/volume",          50).toInt();
    int sfxVol    = cfg.value("sound/effectsVolume",   50).toInt();

    audioOutput->setVolume(bgmVol / 100.0);
    backgroundPlayer->setSource(QUrl("qrc:/bgm/partida.mp3"));
    backgroundPlayer->setLoops(QMediaPlayer::Infinite);
    backgroundPlayer->play();

    //
    // ——— SFX DE ROBA CARTA ———
    //
    effectOutput = new QAudioOutput(this);
    effectPlayer = new QMediaPlayer(this);
    effectPlayer->setAudioOutput(effectOutput);

    effectOutput->setVolume(sfxVol / 100.0);
    effectPlayer->setSource(QUrl("qrc:/bgm/card_draw.mp3"));
}

/**
 * @brief Inicializa la conexión WebSocket y los handlers de eventos.
 */
void EstadoPartida::init() {
    // WebSockets
    websocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(websocket, &QWebSocket::connected, this, [=]() {
        qDebug() << "WebSocket conectado.";
    });

    connect(websocket, &QWebSocket::textMessageReceived, this, [=](const QString& mensaje) {
        this->procesarMensajeWebSocket(mensaje);
    });

    connect(websocket, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error) {
        qWarning() << "Error en WebSocket:" << websocket->errorString();
    });

    qDebug() << "Conectando a:" << wsUrl;
    websocket->open(QUrl(wsUrl));
}

/**
 * @brief Destructor de EstadoPartida.
 *
 * Se encarga de limpiar recursos y desconectar la partida.
 */
EstadoPartida::~EstadoPartida() {
    qDebug() << "[DEBUG] EstadoPartida destruido.";

    // Parar y liberar BGM
    if (backgroundPlayer) {
        backgroundPlayer->stop();
        backgroundPlayer->deleteLater();
        backgroundPlayer = nullptr;
    }
    // Parar y liberar SFX
    if (effectPlayer) {
        effectPlayer->stop();
        effectPlayer->deleteLater();
        effectPlayer = nullptr;
    }
    // Los QAudioOutput se borran automáticamente como hijos de this

    this->limpiar();
}


/**
 * @brief Limpia todos los widgets y datos asociados a la partida.
 *
 * Elimina manos, mazo, carta de triunfo y overlays.
 */
void EstadoPartida::limpiar() {
    // Borrar widgets de mano y sus datos
    for (Jugador* jugador : jugadores) {
        if (jugador->mano) {
            jugador->mano->hide();
            jugador->mano->deleteLater();
            jugador->mano = nullptr;
        }
        delete jugador;
    }
    jugadores.clear();
    mapJugadores.clear();

    // Limpiar cartaTriunfo si existe
    if (cartaTriunfo) {
        cartaTriunfo->hide();
        cartaTriunfo->deleteLater();
        cartaTriunfo = nullptr;
    }

    // Limpiar mazo si existe
    if (mazo) {
        mazo->hide();
        mazo->deleteLater();
        mazo = nullptr;
    }

    // Limpiar overlay
    if (overlay) {
        overlay->hide();
        overlay->deleteLater();
        overlay = nullptr;
        overlayMsg = nullptr;
    }
}

/**
 * @brief Actualiza el estado de la partida según el JSON recibido.
 *
 * Crea jugadores, distribuye cartas y prepara subcomponentes.
 *
 * @param data Objeto JSON con los datos de la partida.
 */
void EstadoPartida::actualizarEstado(const QJsonObject& data) {
    limpiar();

    QJsonArray jugadoresJson = data.value("jugadores").toArray();
    for(const QJsonValue& val : jugadoresJson) {
        QJsonObject obj = val.toObject();
        Jugador* j = new Jugador;
        j->id = obj["id"].toInt();
        j->nombre = obj["nombre"].toString();
        j->equipo = obj["equipo"].toInt();
        j->numCartas = obj["num_cartas"].toInt();
        jugadores.append(j);
        mapJugadores[j->id] = j;
    }

    chatId = data.value("chat_id").toInt();
    mazoRestante = data.value("mazo_restante").toInt();

    QJsonObject triunfo = data.value("carta_triunfo").toObject();
    QString palo = triunfo["palo"].toString();
    QString valor = QString::number(triunfo["valor"].toInt());
    cartaTriunfo = new Carta(palo, valor, this);
    cartaTriunfo->show();

    if(mazoRestante > 1) {
        mazo = new Carta(this);
        mazo->show();
    }

    Jugador* yo = mapJugadores.value(miId, nullptr);
    if(!yo) return;

    yo->mano = new Mano(Orientacion::DOWN, this, this);
    QJsonArray misCartas = data.value("mis_cartas").toArray();
    for(const QJsonValue& cartaVal : misCartas) {
        QJsonObject cartaObj = cartaVal.toObject();
        QString palo = cartaObj["palo"].toString();
        QString valor = QString::number(cartaObj["valor"].toInt());
        yo->mano->agnadirCarta(new Carta(palo, valor));
    }

    if(jugadores.size() == 2) {
        for(Jugador* j : jugadores) {
            if(j->id != miId) {
                j->mano = new Mano(Orientacion::TOP, this, this);
                for(int i = 0; i < j->numCartas; ++i)
                    j->mano->agnadirCarta(new Carta());
            }
        }
    } else {
        Orientacion orient;
        int k = 0;
        for(Jugador* j : jugadores) {
            if(j->id != miId) {
                if(j->equipo == yo->equipo) {
                    orient = Orientacion::TOP;
                } else {
                    orient = (k++ == 0) ? Orientacion::LEFT : Orientacion::RIGHT;
                }
                j->mano = new Mano(orient, this, this);
                for(int i = 0; i < j->numCartas; ++i)
                    j->mano->agnadirCarta(new Carta());
            }
            j->mano->ocultarCartaJugada();
        }
    }
}

/**
 * @brief Establece el identificador del jugador local.
 * @param id Identificador único del jugador.
 */
void EstadoPartida::setMiId(int id) {
    miId = id;
}

/**
 * @brief Marca si la partida ha comenzado.
 * @param iniciada True si la partida está en curso.
 */
void EstadoPartida::setPartidaIniciada(bool iniciada) {
    partidaIniciada = iniciada;
}

/**
 * @brief Consulta si la partida se encuentra iniciada.
 * @return True si la partida está activa.
 */
bool EstadoPartida::getPartidaIniciada() const {
    return partidaIniciada;
}

/**
 * @brief Dibuja y posiciona todos los elementos visuales de la partida.
 *
 * Coloca manos, mazo, carta triunfo, puntuaciones y botones.
 */
void EstadoPartida::dibujarEstado() {

    QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();
    int width = screenSize.width(), height = screenSize.height();

    Jugador* yo = mapJugadores.value(miId, nullptr);
    if(!yo || !yo->mano) return;

    for(Jugador* j : jugadores) {
        if(j->mano) j->mano->dibujar();
    }

    // 2 jugadores (1vs1)
    if(jugadores.size() == 2) {
        Jugador* oponente = (jugadores[0]->id == miId) ? jugadores[1] : jugadores[0];

        yo->mano->move(width/2 - yo->mano->width()/2, height - yo->mano->height());
        oponente->mano->move(width/2 - oponente->mano->width() / 2, 0);
    }
    // 4 jugadores (2vs2)
    else if(jugadores.size() == 4) {
        Jugador* companero = nullptr;
        QVector<Jugador*> enemigos;
        for (Jugador* j : jugadores) {
            if (j->id != miId) {
                if (j->equipo == yo->equipo) companero = j;
                else enemigos.append(j);
            }
        }

        yo->mano->move(width/2 - yo->mano->width()/2, height - yo->mano->height() - 24);
        companero->mano->move(width/2 - companero->mano->width()/2, 24);
        enemigos[0]->mano->move(24, height/2 - enemigos[0]->mano->height()/2);
        enemigos[1]->mano->move(width - enemigos[1]->mano->width() - 24, height/2 - enemigos[1]->mano->height()/2);
    }

    // Posicionar carta de triunfo
    if (cartaTriunfo && cartaTriunfo->isVisible()) {
        if (mazoRestante > 1 || mazo->isVisible())
            cartaTriunfo->move(width/2 - cartaTriunfo->width() - 10, height/2 - cartaTriunfo->height()/2);
        else
            cartaTriunfo->move(width/2 - cartaTriunfo->width()/2, height/2 - cartaTriunfo->height()/2);
        cartaTriunfo->raise();
    }

    // Baraja central (si quedan cartas)
    if (mazoRestante > 1) {
        if (mazo->isVisible()) {
            mazo->move(width/2 + 10, height/2 - mazo->height()/2);
            mazo->raise();
        }
    }

    // Posicionar puntuaciones
    puntosEquipo1Title->move(2 * width/6 - puntosEquipo1Title->width()/2, height/2 - puntosEquipo1Title->height()/2 - puntosEquipo1Label->height());
    puntosEquipo1Title->show();

    puntosEquipo1Label->move(2 * width/6 - puntosEquipo1Label->width()/2, height/2 - puntosEquipo1Label->height()/2);
    puntosEquipo1Label->setText(QString::number(puntosEquipo1));
    puntosEquipo1Label->show();

    puntosEquipo2Title->move(4 * width/6 - puntosEquipo2Title->width()/2, height/2 - puntosEquipo2Title->height()/2 - puntosEquipo2Label->height());
    puntosEquipo2Title->show();

    puntosEquipo2Label->move(4 * width/6 - puntosEquipo2Label->width()/2, height/2 - puntosEquipo2Label->height()/2);
    puntosEquipo2Label->setText(QString::number(puntosEquipo2));
    puntosEquipo2Label->show();

    // Botones cantar y cambiar siete
    if(botonCantar && botonCambiarSiete) {
        int x = puntosEquipo1Label->pos().x() + puntosEquipo1Label->width()/2 - botonCantar->width()/2;
        int y = yo->mano->pos().y() - botonCantar->height()/2;
        botonCantar->move(x, y);
        botonCambiarSiete->move(x, y + botonCantar->height() + 24);
    }

    // Solicitar pausa o reanudar boton
    if(botonPausa) {
        int x = puntosEquipo2Label->pos().x() + puntosEquipo2Label->width()/2;
        int y = yo->mano->pos().y() - botonPausa->height()/2;
        botonPausa->move(x - botonPausa->width()/2, y);
        pausadosLabel->move(x - pausadosLabel->width()/2, y + botonPausa->height() + 24);
    }

    if (botonCantar) {
        botonCantar->show();
        botonCantar->raise();
    }
    if (botonCambiarSiete) {
        botonCambiarSiete->show();
        botonCambiarSiete->raise();
    }
    if (botonPausa) {
        botonPausa->show();
        botonPausa->raise();
    }
    if (pausadosLabel) {
        pausadosLabel->show();
        pausadosLabel->raise();
    }
}

/**
 * @brief Anima y actualiza la puntuación de un equipo.
 *
 * Muestra una transición numérica hasta el nuevo valor.
 *
 * @param equipo Número de equipo (1 o 2).
 * @param nuevoValor Valor destino de la puntuación.
 * @param callback Llamada al finalizar la animación.
 */
void EstadoPartida::actualizarPuntuacion(int equipo, int nuevoValor, std::function<void()> callback) {
    QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();
    int width = screenSize.width(), height = screenSize.height();

    QLabel* label = (equipo == 1) ? puntosEquipo1Label : puntosEquipo2Label;
    int* valorPtr = (equipo == 1) ? &puntosEquipo1 : &puntosEquipo2;

    if(!label || nuevoValor == *valorPtr) {
        if (callback) callback();
        return;
    }

    int paso = (nuevoValor > *valorPtr) ? 1 : -1;
    QTimer* timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [=]() mutable {
        *valorPtr += paso;
        label->setText(QString::number(*valorPtr));
        label->adjustSize();
        if (equipo == 1) label->move(2 * width / 6 - label->width() / 2, height / 2 - label->height() / 2);
        else label->move(4 * width / 6 - label->width() / 2, height / 2 - label->height() / 2);
        if (*valorPtr == nuevoValor) {
            timer->stop();
            timer->deleteLater();
            if(equipo == 1) puntosEquipo1 = nuevoValor;
            else puntosEquipo2 = nuevoValor;
            if (callback) callback();
        }
    });

    timer->start();
}

/**
 * @brief Maneja el doble clic sobre una carta para jugarla.
 *
 * Envía la acción 'jugar_carta' al servidor.
 *
 * @param carta Puntero a la carta seleccionada.
 */
void EstadoPartida::onCartaDobleClick(Carta* carta) {
    if(websocket) {
        QJsonObject msg;
        msg["accion"] = "jugar_carta";
        QJsonObject cartaJson;
        cartaJson["palo"] = carta->getPalo();
        bool ok;
        cartaJson["valor"] = carta->getValor().toInt(&ok);
        if(!ok) cartaJson["valor"] = 0;
        msg["carta"] = cartaJson;

        this->enviarMsg(
            msg
        );
    }
    qDebug() << "Jugar carta";
}

/**
 * @brief Envía la acción 'cantar' al servidor.
 */
void EstadoPartida::onCantar() {
    if(websocket) {
        QJsonObject msg;
        msg["accion"] = "cantar";
        this->enviarMsg(
            msg
        );
    }
    qDebug() << "Cantar";
}

/**
 * @brief Envía la acción 'cambiar_siete' al servidor.
 */
void EstadoPartida::onCambiarSiete() {
    if(websocket) {
        QJsonObject msg;
        msg["accion"] = "cambiar_siete";
        this->enviarMsg(
            msg
        );
    }
    qDebug() << "Cambiar siete";
}


/**
 * @brief Envía la acción 'pausa' al servidor para solicitar pausa.
 */
void EstadoPartida::onPausa() {
    if(websocket) {
        QJsonObject msg;
        msg["accion"] = "pausa";
        this->enviarMsg(
            msg
        );
    }
    qDebug() << "Pausa";
}

/**
 * @brief Envía la acción 'anular_pausa' al servidor.
 */
void EstadoPartida::onAnularPausa() {
    if(websocket) {
        QJsonObject msg;
        msg["accion"] = "anular_pausa";
        this->enviarMsg(
            msg
        );
    }
    qDebug() << "Reanudar";
}

/**
 * @brief Envía un mensaje JSON al servidor vía WebSocket.
 *
 * @param msg Objeto JSON a transmitir.
 */
void EstadoPartida::enviarMsg(QJsonObject& msg) {
    if(websocket) {
        QJsonDocument doc(msg);
        QByteArray msgByteArray = doc.toJson();
        websocket->sendTextMessage(QString::fromUtf8(msgByteArray));
    }
}

//////////////////////////////////////////////////////////////////////////////////////
/// Cola de eventos
//////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Encola un evento recibido para su procesamiento.
 *
 * @param evento Objeto JSON con el evento.
 */
void EstadoPartida::recibirEvento(const QJsonObject& evento) {
    colaEventos.enqueue(evento);
    procesarSiguienteEvento();
}

/**
 * @brief Procesa el siguiente evento en cola si no hay otro en ejecución.
 */
void EstadoPartida::procesarSiguienteEvento() {
    if (enEjecucion || colaEventos.isEmpty())
        return;

    enEjecucion = true;
    QJsonObject evento = colaEventos.dequeue();
    QString tipo = evento["type"].toString();
    QJsonObject data = evento["data"].toObject();

    qDebug() << "Recibido " + tipo;

    if(tipo == "start_game") {
        this->setPartidaIniciada(true);
        procesarStartGame(data);
    } else if (tipo == "card_played") {
        procesarCardPlayed(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if (tipo == "card_drawn") {
        procesarCardDrawn(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if (tipo == "turn_update") {
        procesarTurnUpdate(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if (tipo == "round_result") {
        procesarRoundResult(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if (tipo == "phase_update") {
        procesarPhaseUpdate(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if (tipo == "pause") {
        procesarPause(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if(tipo == "resume") {
        procesarResume(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else if (tipo == "end_game") {
        procesarEndGame(data, nullptr);
    } else if (tipo == "all_pause") {
        procesarAllPause(data, nullptr);
    } else if(tipo == "error") {
        procesarError(data, [this]() {
            enEjecucion = false;
            procesarSiguienteEvento();
        });
    } else {
        enEjecucion = false;
        procesarSiguienteEvento();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
/// Procesar mensajes
//////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Procesa un evento de tipo 'card_played' con animación.
 *
 * @param data Datos del evento en formato JSON.
 * @param callback Función a ejecutar una vez completada la animación.
 */
void EstadoPartida::procesarCardPlayed(QJsonObject data, std::function<void()> callback) {
    int jugadorId = data["jugador"].toObject()["id"].toInt();
    QJsonObject cartaJson = data["carta"].toObject();
    QString paloJugado = cartaJson["palo"].toString();
    QString valorJugado = QString::number(cartaJson["valor"].toInt());

    Jugador* jugador = mapJugadores.value(jugadorId, nullptr);
    if(!jugador || !jugador->mano) return;

    Carta* cartaParaAnimar = nullptr;
    QPoint origenAnimacion;

    // Yo juego la carta
    if(jugadorId == miId) {
        cartaParaAnimar = jugador->mano->extraerCarta(paloJugado, valorJugado);
        cartaParaAnimar->interactuable = false;
        if(cartaParaAnimar) {
            origenAnimacion = cartaParaAnimar->mapToGlobal(QPoint(0, 0));
            cartaParaAnimar->setParent(this);
        } else {
            cartaParaAnimar = new Carta(paloJugado, valorJugado, this);
            origenAnimacion = jugador->mano->mapToGlobal(
                QPoint(jugador->mano->width()/2, jugador->mano->height()/2)
            );
        }
    }
    // Juega otro jugador
    else {
        Carta* cartaPlaceholder = jugador->mano->pop();
        if(cartaPlaceholder) {
            origenAnimacion = cartaPlaceholder->mapToGlobal(QPoint(0, 0));
            cartaPlaceholder->hide();
            cartaPlaceholder->deleteLater();
        } else {
            origenAnimacion = jugador->mano->mapToGlobal(
                QPoint(jugador->mano->width()/2, jugador->mano->height()/2)
            );
        }
        cartaParaAnimar = new Carta(paloJugado, valorJugado, this);
    }

    // Si algo va mal
    if(!cartaParaAnimar) {
        jugador->mano->actualizarCartaJugada(paloJugado, valorJugado);
        jugador->mano->dibujar();
        this->dibujarEstado();
        return;
    }

    // Animar
    QPoint origenAnimacionLocal = this->mapFromGlobal(origenAnimacion);
    cartaParaAnimar->setOrientacion(jugador->mano->getOrientacion());
    cartaParaAnimar->move(origenAnimacionLocal);
    cartaParaAnimar->show();
    cartaParaAnimar->raise();
    QPoint destinoAnimacion = jugador->mano->mapToGlobal(jugador->mano->getZonaDeJuego());
    QPoint destinoAnimacionLocal = this->mapFromGlobal(destinoAnimacion);

    QPropertyAnimation* anim = new QPropertyAnimation(cartaParaAnimar, "pos", this);
    anim->setDuration(500);
    anim->setStartValue(origenAnimacionLocal);
    anim->setEndValue(destinoAnimacionLocal);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QPropertyAnimation::finished, this, [=]() {
        jugador->mano->actualizarCartaJugada(paloJugado, valorJugado);
        cartaParaAnimar->hide();
        cartaParaAnimar->deleteLater();
        jugador->mano->dibujar();
        this->dibujarEstado();
        if (callback) callback();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief Procesa un evento de tipo 'card_drawn', reparte cartas con animaciones.
 *
 * @param data Datos del evento en formato JSON.
 * @param callback Función a ejecutar tras finalizar todas las animaciones.
 */
void EstadoPartida::procesarCardDrawn(QJsonObject data, std::function<void()> callback) {

    QJsonObject cartaJson = data.value("carta").toObject();
    QString palo = cartaJson["palo"].toString();
    QString valor = QString::number(cartaJson["valor"].toInt());

    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);

    // Actualizar cartas restantes en mazo central
    mazoRestante -= jugadores.size();
    if(mazoRestante <= 0) mazo->hide();

    // Actualizar manos
    for(Jugador* jugador : jugadores) {
        if(!jugador || !jugador->mano || jugador->mano->getNumCartas() >= 6)
            continue;

        Carta* carta = new Carta();
        if(jugador->id == miId) carta = new Carta(palo, valor);

        QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(carta); // Parent effect to the card
        opacityEffect->setOpacity(0.0); // Start completely transparent
        carta->setGraphicsEffect(opacityEffect);

        jugador->mano->agnadirCarta(carta);
        this->dibujarEstado();

        QPropertyAnimation* anim = new QPropertyAnimation(opacityEffect, "opacity", animationGroup);
        anim->setDuration(1000);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);;
        anim->setEasingCurve(QEasingCurve::InCubic);
        animationGroup->addAnimation(anim);
    }

    connect(animationGroup, &QParallelAnimationGroup::finished, this, [=]() {
        if(mazoRestante <= jugadores.size()) {
            mazo->show(); cartaTriunfo->show();

            QGraphicsOpacityEffect* mazoEffect = new QGraphicsOpacityEffect(mazo);
            mazo->setGraphicsEffect(mazoEffect);
            mazoEffect->setOpacity(1.0);

            QPropertyAnimation* fadeOutMazo = new QPropertyAnimation(mazoEffect, "opacity");
            fadeOutMazo->setDuration(1000);
            fadeOutMazo->setStartValue(1.0);
            fadeOutMazo->setEndValue(0.0);
            fadeOutMazo->setEasingCurve(QEasingCurve::OutCubic);

            QPoint centro(this->width() / 2 - cartaTriunfo->width() / 2,
                          this->height() / 2 - cartaTriunfo->height() / 2);

            QPropertyAnimation* moverTriunfo = new QPropertyAnimation(cartaTriunfo, "pos");
            moverTriunfo->setDuration(1000);
            moverTriunfo->setStartValue(cartaTriunfo->pos());
            moverTriunfo->setEndValue(centro);
            moverTriunfo->setEasingCurve(QEasingCurve::OutCubic);

            QParallelAnimationGroup* finalGroup = new QParallelAnimationGroup(this);
            finalGroup->addAnimation(fadeOutMazo);
            finalGroup->addAnimation(moverTriunfo);

            connect(finalGroup, &QParallelAnimationGroup::finished, this, [=]() {
                mazo->hide();
                mazo->setGraphicsEffect(nullptr);
                this->dibujarEstado();
                if (callback) callback();
            });

            finalGroup->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            this->dibujarEstado();
            if (callback) callback();
        }
    });

    if(animationGroup->animationCount() > 0) {
        animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        delete animationGroup;
        this->dibujarEstado();
    }
    if (effectPlayer) {
        effectPlayer->stop();
        effectPlayer->play();
    }
}

/**
 * @brief Inicializa los botones de acción y etiquetas de puntuación.
 */
void EstadoPartida::iniciarBotonesYEtiquetas() {
    // Puntuaciones
    puntosEquipo1Title = new QLabel("Pt. Equipo 1", this);
    puntosEquipo1Title->setStyleSheet("font-size: 32px; color: white; background-color: transparent; border: none;");
    puntosEquipo1Title->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    puntosEquipo1Label = new QLabel(this);
    puntosEquipo1Label->setStyleSheet("font-size: 64px; font-weight: bold; color: white; background-color: transparent; border: none;");
    puntosEquipo1Label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    puntosEquipo2Title = new QLabel("Pt. Equipo 2", this);
    puntosEquipo2Title->setStyleSheet("font-size: 32px; color: white; background-color: transparent; border: none;");
    puntosEquipo2Title->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    puntosEquipo2Label = new QLabel(this);
    puntosEquipo2Label->setStyleSheet("font-size: 64px; font-weight: bold; color: white; background-color: transparent; border: none;");
    puntosEquipo2Label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Botones acciones
    botonCantar = new BotonAccion("Cantar", [this]() {
        this->onCantar();
    }, this);

    botonCambiarSiete = new BotonAccion("Cambiar siete", [this]() {
        this->onCambiarSiete();
    }, this);

    QString textoPausa = enPausa ? "Anular pausa" : "Solicitar pausa";
    botonPausa = new BotonAccion(textoPausa, [this]() {
        if(enPausa) this->onAnularPausa();
        else this->onPausa();
    }, this);

    pausadosLabel = new QLabel("0/0", this);
    pausadosLabel->setFixedSize(68, 68);
    pausadosLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #1f1f1f;"
        "   color: white;"
        "   border-radius: 34px;"
        "   font-size: 24px;"
        "   border: 2px solid #666;"
        "   qproperty-alignment: AlignCenter;"
        "}"
        );
    pausadosLabel->setAlignment(Qt::AlignCenter);
}

/**
 * @brief Procesa el inicio de partida ('start_game').
 *
 * @param data Datos iniciales de la partida.
 */
void EstadoPartida::procesarStartGame(QJsonObject data) {
    this->setPartidaIniciada(true);
    ocultarOverlayEspera();

    this->iniciarBotonesYEtiquetas();
    this->actualizarEstado(data);
    this->dibujarEstado();

    QTimer::singleShot(250, this, [=]() {
        this->dibujarEstado();
        enEjecucion = false;
        procesarSiguienteEvento();
    });
}

/**
 * @brief Procesa actualización de turno ('turn_update'), muestra mensaje.
 *
 * @param data Datos del turno en JSON.
 * @param callback Función tras cerrar el mensaje.
 */
void EstadoPartida::procesarTurnUpdate(QJsonObject data, std::function<void()> callback) {
    QString mensaje = data.value("message").toString("Turno desconocido");
    this->mostrarMensaje(mensaje, callback);
}

/**
 * @brief Procesa fin de partida ('end_game'), muestra ganador.
 *
 * @param data Datos finales de la partida.
 * @param callback Función opcional para después del mensaje.
 */
void EstadoPartida::procesarEndGame(QJsonObject data, std::function<void()> /*callback*/) {
    int ganador = data.value("ganador_equipo").toInt();
    int puntos1 = data.value("puntos_equipo_1").toInt();
    int puntos2 = data.value("puntos_equipo_2").toInt();

    QString mensaje;
    if (ganador == 0)
        mensaje = QString("¡Empate!\n%1 - %2").arg(puntos1).arg(puntos2);
    else
        mensaje = QString("¡Ha ganado el equipo %1!\n%2 - %3").arg(ganador).arg(puntos1).arg(puntos2);

    VentanaInfo* info = new VentanaInfo(mensaje, [this](){
        if(this->onSalir) this->onSalir();
        this->deleteLater();
    }, this);
}

/**
 * @brief Procesa cambio de fase ('phase_update'), calcula puntos de triunfo.
 *
 * @param data Datos de la fase.
 * @param callback Función tras actualizar puntuaciones.
 */
void EstadoPartida::procesarPhaseUpdate(QJsonObject data, std::function<void()> callback) {
    this->mostrarMensaje("Cambio a fase de arrastre", [=]() {
        if (!cartaTriunfo) {
            if (callback) callback();
            return;
        }
        Carta* copia = new Carta(cartaTriunfo->getPalo(), cartaTriunfo->getValor(), this);
        QPoint posGlobal = cartaTriunfo->mapToGlobal(QPoint(0, 0));
        copia->move(this->mapFromGlobal(posGlobal));
        copia->show();
        copia->raise();
        cartaTriunfo->hide();

        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(copia);
        copia->setGraphicsEffect(effect);
        QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(1000);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        connect(anim, &QPropertyAnimation::finished, this, [=]() {
            copia->deleteLater();
            int valor = cartaTriunfo->getValor().toInt();
            int puntos = 0;
            switch (valor) {
                case 1:  puntos = 11; break;
                case 3:  puntos = 10; break;
                case 12: puntos = 4; break;
                case 10: puntos = 3; break;
                case 11: puntos = 2; break;
                default: puntos = 0;
            }

            int equipo = data.value("equipo_que_gana_triunfo").toInt();
            int puntosFinales1 = puntosEquipo1;
            int puntosFinales2 = puntosEquipo2;

            if (equipo == 1) puntosFinales1 += puntos;
            else if (equipo == 2) puntosFinales2 += puntos;

            if (puntosFinales1 != puntosEquipo1)
                this->actualizarPuntuacion(1, puntosFinales1, callback);
            else
                this->actualizarPuntuacion(2, puntosFinales2, callback);
        });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

/**
 * @brief Procesa resultado de baza ('round_result'), muestra overlay y puntúa.
 *
 * @param data Datos de resultado de la ronda.
 * @param callback Función tras finalizar todos los procesos.
 */
void EstadoPartida::procesarRoundResult(QJsonObject data, std::function<void()> callback) {
    QJsonObject ganador = data.value("ganador").toObject();
    QString nombre = ganador.value("nombre").toString("Desconocido");
    int equipo = ganador.value("equipo").toInt(-1);

    QString mensaje = QString("¡%1 ganó la baza para el equipo %2!")
                          .arg(nombre)
                          .arg(equipo);

    int puntos1 = data.value("puntos_equipo_1").toInt();
    int puntos2 = data.value("puntos_equipo_2").toInt();

    // Mostrar overlay y luego actualizar puntuaciones
    this->mostrarMensaje(mensaje, [=]() {
        int completados = 0;
        auto onDone = [this, callback]() {
            if (callback) callback();
        };

        if (puntos1 != puntosEquipo1)
            this->actualizarPuntuacion(1, puntos1, onDone);
        else
            this->actualizarPuntuacion(2, puntos2, onDone);
    });

    for(Jugador* j : jugadores) {
        if(!j || !j->mano) continue;

        Carta* copia = new Carta(j->mano->jugada()->getPalo(), j->mano->jugada()->getValor(), this);
        QPoint posGlobal = j->mano->jugada()->mapToGlobal(QPoint(0, 0));
        QPoint posLocal = this->mapFromGlobal(posGlobal);
        copia->setOrientacion(j->mano->getOrientacion());
        copia->move(posLocal);
        j->mano->ocultarCartaJugada();
        copia->show();
        copia->raise();

        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(copia);
        copia->setGraphicsEffect(effect);
        QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(1000);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QPropertyAnimation::finished, copia, [copia]() {
            copia->deleteLater();
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

/**
 * @brief Procesa solicitud de pausa ('pause').
 *
 * @param data Datos de la solicitud.
 * @param callback Función tras mostrar mensaje.
 */
void EstadoPartida::procesarPause(QJsonObject data, std::function<void()> callback) {
    int jugadorId = data["jugador"].toObject()["id"].toInt();
    QString nombre = data["jugador"].toObject()["nombre"].toString();
    jugadoresPausa = data["num_solicitudes_pausa"].toInt();
    if(jugadorId == miId) {
        enPausa = true;
        if(botonPausa) botonPausa->setText("Anular pausa");
    }
    pausadosLabel->setText(QString("%1/%2").arg(jugadoresPausa).arg(jugadores.size()));
    QString msg = QString("%1 ha solicitado pausa").arg(nombre);
    mostrarMensaje(msg, callback);
}

/**
 * @brief Procesa anulación de pausa ('resume').
 *
 * @param data Datos de la anulación.
 * @param callback Función tras mostrar mensaje.
 */
void EstadoPartida::procesarResume(QJsonObject data, std::function<void()> callback) {
    int jugadorId = data["jugador"].toObject()["id"].toInt();
    QString nombre = data["jugador"].toObject()["nombre"].toString();
    jugadoresPausa = data["num_solicitudes_pausa"].toInt();
    if(jugadorId == miId) {
        enPausa = false;
        if(botonPausa) botonPausa->setText("Solicitar pausa");
    }
    pausadosLabel->setText(QString("%1/%2").arg(jugadoresPausa).arg(jugadores.size()));
    QString msg = QString("%1 ha anulado su solicitud de pausa").arg(nombre);
    mostrarMensaje(msg, callback);
}

/**
 * @brief Procesa pausa global ('all_pause'), finaliza la partida.
 *
 * @param data Datos del evento.
 * @param callback Función opcional tras mostrar mensaje.
 */
void EstadoPartida::procesarAllPause(QJsonObject data, std::function<void()> callback) {
    QString mensaje = "La partida ha sido pausada";
    VentanaInfo* info = new VentanaInfo(mensaje, [this](){
        if(this->onSalir) this->onSalir();
        this->deleteLater();
    }, this);
}

/**
 * @brief Procesa un error recibido del servidor ('error').
 *
 * @param data Datos del error.
 * @param callback Función tras cerrar el popup.
 */
void EstadoPartida::procesarError(QJsonObject data, std::function<void()> callback) {
    auto *popup = new QLabel(data.value("message").toString("Error desconocido"), this);
    popup->setStyleSheet("QLabel {"
                         "background: rgba(50,50,50,220);"
                         "color: white; font-size: 18px;"
                         "padding: 6px 12px;"
                         "border-radius: 6px; border: 1px solid #888; }"
    );
    popup->adjustSize();
    popup->move(this->mapFromGlobal(QCursor::pos()));
    popup->show();

    auto *effect = new QGraphicsOpacityEffect(popup);
    popup->setGraphicsEffect(effect);

    auto *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    auto *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(700);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    auto *group = new QSequentialAnimationGroup(this);
    group->addAnimation(fadeIn);
    group->addPause(1500);
    group->addAnimation(fadeOut);

    connect(group, &QSequentialAnimationGroup::finished, this, [popup, callback]() {
        popup->deleteLater();
        if (callback) callback();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}


//////////////////////////////////////////////////////////////////////////////////////
/// Overlay
//////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Muestra un mensaje superpuesto con opacidad animada.
 *
 * @param msg Texto a mostrar.
 * @param callback Función tras desaparecer el mensaje.
 * @param duracion Tiempo de visualización en milisegundos (por defecto).
 */
void EstadoPartida::mostrarMensaje(const QString& msg, std::function<void()> callback, int duracion) {
    if(overlay) {
        delete overlay;
        overlay = nullptr;
        overlayMsg = nullptr;
    }

    // Fondo
    overlay = new QWidget(this);
    overlay->setGeometry(this->geometry());
    overlay->setStyleSheet("QWidget { background-color: black; }");
    overlay->raise();

    // Mensaje
    overlayMsg = new QLabel(msg, overlay);
    overlayMsg->setAlignment(Qt::AlignCenter);
    overlayMsg->setStyleSheet("QLabel { color: white; font-size: 64px; font-weight: bold; }");

    QVBoxLayout* layout = new QVBoxLayout(overlay);
    layout->addWidget(overlayMsg, 0, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    overlay->setLayout(layout);
    overlay->show();

    // Animación
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(overlay);
    opacityEffect->setOpacity(0.0);
    overlay->setGraphicsEffect(opacityEffect);

    overlay->show();
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    QPropertyAnimation* fadeInAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeInAnim->setDuration(500);
    fadeInAnim->setStartValue(0.0);
    fadeInAnim->setEndValue(0.8);
    fadeInAnim->setEasingCurve(QEasingCurve::OutCubic);
    QPauseAnimation* holdAnim = new QPauseAnimation(duracion);
    QPropertyAnimation* fadeOutAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOutAnim->setDuration(500);
    fadeOutAnim->setStartValue(0.8);
    fadeOutAnim->setEndValue(0.0);
    fadeOutAnim->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(fadeInAnim);
    group->addAnimation(holdAnim);
    group->addAnimation(fadeOutAnim);

    connect(group, &QSequentialAnimationGroup::finished, this, [this, callback]() {
        if (overlay) {
            delete overlay;
            overlay = nullptr;
            overlayMsg = nullptr;
        }
        if(callback) callback();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}


//////////////////////////////////////////////////////////////////////////////////////
/// Overlay espera
//////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Muestra un overlay mientras llegan jugadores ('player_joined'/'player_left').
 *
 * @param jugadoresCola Jugadores actualmente en cola.
 * @param jugadoresMax Jugadores requeridos.
 */
void EstadoPartida::mostrarOverlayEspera(int jugadoresCola, int jugadoresMax) {
    if (overlayEspera) {
        overlayEspera->deleteLater();
        overlayEspera = nullptr;
        labelEspera = nullptr;
    }

    overlayEspera = new QWidget(this);
    QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();
    overlayEspera->setGeometry(0, 0, screenSize.width(), screenSize.height());
    overlayEspera->setStyleSheet("background-color: rgba(0, 0, 0, 160);");
    overlayEspera->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlayEspera->lower();
    overlayEspera->setAttribute(Qt::WA_DeleteOnClose);

    labelEspera = new QLabel(QString("Esperando jugadores... (%1/%2)").arg(jugadoresCola).arg(jugadoresMax), overlayEspera);
    labelEspera->setAlignment(Qt::AlignCenter);
    labelEspera->setStyleSheet("color: white; font-size: 40px; font-weight: bold;");

    QVBoxLayout* layout = new QVBoxLayout(overlayEspera);
    layout->addWidget(labelEspera);
    overlayEspera->setLayout(layout);
    overlayEspera->show();
}

/**
 * @brief Actualiza el texto del overlay de espera.
 *
 * @param jugadoresCola Jugadores actualmente en cola.
 * @param jugadoresMax Jugadores requeridos.
 */
void EstadoPartida::actualizarOverlayEspera(int jugadoresCola, int jugadoresMax) {
    if (labelEspera) {
        labelEspera->setText(QString("Esperando jugadores... (%1/%2)").arg(jugadoresCola).arg(jugadoresMax));
    }
}

/**
 * @brief Oculta y destruye el overlay de espera.
 */
void EstadoPartida::ocultarOverlayEspera() {
    if (overlayEspera) {
        overlayEspera->deleteLater();
        overlayEspera = nullptr;
        labelEspera = nullptr;
    }
}

/**
 * @brief Maneja eventos antes del inicio de partida.
 *
 * Procesa 'player_joined' y 'player_left'.
 *
 * @param tipo Tipo de evento.
 * @param data Datos del evento en JSON.
 */
void EstadoPartida::manejarEventoPrePartida(const QString& tipo, const QJsonObject& data) {
    if (tipo == "player_joined" || tipo == "player_left") {
        int jugadoresCola = data.value("jugadores").toInt();
        int jugadoresMax = data.value("capacidad").toInt();

        if(tipo == "player_joined") {
            QString nom = data.value("usuario").toObject().value("nombre").toString();
            int id = data.value("usuario").toObject().value("id").toInt();
            if(nom == miNombre) {
                this->setMiId(id);
            }
        }

        if (!overlayEspera) {
            mostrarOverlayEspera(jugadoresCola, jugadoresMax);
        } else {
            actualizarOverlayEspera(jugadoresCola, jugadoresMax);
        }
    }
}

/**
 * @brief Procesa mensajes entrantes de WebSocket.
 *
 * Enruta a pre-partida o al sistema de eventos.
 *
 * @param mensaje Cadena JSON recibida.
 */
void EstadoPartida::procesarMensajeWebSocket(const QString& mensaje) {
    QJsonDocument doc = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!doc.isObject()) return;
    QJsonObject root = doc.object();
    QString tipo = root["type"].toString();
    QJsonObject data = root["data"].toObject();

    if (!this->getPartidaIniciada() && (tipo == "player_joined" || tipo == "player_left")) {
        this->manejarEventoPrePartida(tipo, data);
    } else {
        this->recibirEvento(root);
    }
}

void EstadoPartida::setVolume(int volumePercentage) {
    if (audioOutput) {
        audioOutput->setVolume(volumePercentage / 100.0);
    }
}
