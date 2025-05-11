#include "estadopartida.h"
#include "settingswindow.h"
#include "gamemessagewindow.h"
#include "menuwindow.h"
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QJsonArray>
#include <QScreen>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPushButton>
#include <QDialog>
#include <QJsonDocument>
#include "ventanasalirpartida.h"

EstadoPartida::EstadoPartida(QString miNombre, QString userKey,  MenuWindow* menu, const QString& wsUrl, int bg, int style,
                             std::function<void()> onSalir, QWidget* parent)
    : QWidget(parent), miNombre(miNombre), onSalir(onSalir), wsUrl(wsUrl) {
    this->userKey = userKey;
    menuWindowRef = menu;
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
}

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

EstadoPartida::~EstadoPartida() {
    qDebug() << "[DEBUG] EstadoPartida destruido.";
    this->limpiar();
}

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
    if (optionsBar) {
        optionsBar->hide();
        optionsBar->deleteLater();
        optionsBar = nullptr;
    }
}

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
    crearBarraSuperior();
}

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
    if(optionsBar) {
        optionsBar->show();
        optionsBar->raise();
    }
}

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

void EstadoPartida::recibirEvento(const QJsonObject& evento) {
    colaEventos.enqueue(evento);
    procesarSiguienteEvento();
}

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
}

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

void EstadoPartida::procesarStartGame(QJsonObject data) {
    this->setPartidaIniciada(true);
    ocultarOverlayEspera();

    this->iniciarBotonesYEtiquetas();
    this->actualizarEstado(data);
    this->dibujarEstado();


    QTimer::singleShot(250, this, [=]() {
        this->dibujarEstado();
        this->crearBarraSuperior();
        enEjecucion = false;
        procesarSiguienteEvento();
    });
}

void EstadoPartida::procesarTurnUpdate(QJsonObject data, std::function<void()> callback) {
    QString mensaje = data.value("message").toString("Turno desconocido");
    this->mostrarMensaje(mensaje, callback);
}

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

void EstadoPartida::procesarAllPause(QJsonObject data, std::function<void()> callback) {
    QString mensaje = "La partida ha sido pausada";
    VentanaInfo* info = new VentanaInfo(mensaje, [this](){
        if(this->onSalir) this->onSalir();
        this->deleteLater();
    }, this);
}

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

void EstadoPartida::actualizarOverlayEspera(int jugadoresCola, int jugadoresMax) {
    if (labelEspera) {
        labelEspera->setText(QString("Esperando jugadores... (%1/%2)").arg(jugadoresCola).arg(jugadoresMax));
    }
}

void EstadoPartida::ocultarOverlayEspera() {
    if (overlayEspera) {
        overlayEspera->deleteLater();
        overlayEspera = nullptr;
        labelEspera = nullptr;
    }
}

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

// En tu clase EstadoPartida

void EstadoPartida::crearBarraSuperior() {
    // Barra superior
    optionsBar = new QFrame(this);
    optionsBar->setObjectName("optionsBar");
    optionsBar->setFixedHeight(60);  // Hacer la barra más corta
    optionsBar->setGeometry((this->width() - 300) / 2, 0, 300, 60); // Centramos la barra en la parte superior

    optionsBar->installEventFilter(this);

    // Estilo de la barra
    optionsBar->setStyleSheet(R"(
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

    // Layout para centrar los iconos
    QHBoxLayout *layout = new QHBoxLayout(optionsBar);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(10, 0, 10, 0); // Márgenes
    layout->setSpacing(20);

    // Crear los iconos y añadirlos al layout
    settings = new Icon(optionsBar);
    settings->setImage(":/icons/audio.png", 50, 50);

    chat = new Icon(optionsBar);
    chat->setImage(":/icons/message.png", 50, 50);

    quit = new Icon(optionsBar);
    quit->setImage(":/icons/door.png", 60, 60);

    // Añadir los iconos al layout
    layout->addWidget(settings);
    layout->addWidget(chat);
    layout->addWidget(quit);

    // Conexiones de los iconos
    connect(settings, &Icon::clicked, this, [=]() {
        settings->setImage(":/icons/darkenedaudio.png", 50, 50);
        SettingsWindow *w = new SettingsWindow(menuWindowRef, this, miNombre);
        w->setModal(true);
        connect(w, &QDialog::finished, [=](int){ settings->setImage(":/icons/audio.png", 50, 50); });
        w->exec();
    });

    connect(chat, &Icon::clicked, this, [this]() {
        GameMessageWindow *w = new GameMessageWindow(userKey, this, QString::number(chatId), QString::number(miId));
        w->setWindowModality(Qt::ApplicationModal);
        w->move(this->geometry().center() - w->rect().center());
        w->show(); w->raise(); w->activateWindow();
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
            confirmDialog->close();
            QSize windowSize = this->size();
            MenuWindow *menuWindow = new MenuWindow(userKey);
            menuWindow->resize(windowSize);
            menuWindow->show();
            menuWindow->raise();
            menuWindow->activateWindow();
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

        QTimer *centerTimer = new QTimer(confirmDialog);
        centerTimer->setInterval(50);
        connect(centerTimer, &QTimer::timeout, [this, confirmDialog]() {
            confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        });
    });
}


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
