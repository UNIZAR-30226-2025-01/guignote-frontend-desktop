#include "estadopartida.h"
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
#include "ventanasalirpartida.h"

EstadoPartida::EstadoPartida(QString miNombre, const QString &wsUrl, int bg, int style,
                             std::function<void()> onSalir, QWidget *parent)
    : QWidget(parent), miNombre(miNombre), onSalir(onSalir), wsUrl(wsUrl)
{
    if (bg == 0)
    {
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
    }
    else
    {
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

EstadoPartida::~EstadoPartida()
{
    qDebug() << "[DEBUG] EstadoPartida destruido.";
    this->limpiar();
}

void EstadoPartida::limpiar()
{
    // Borrar widgets de mano y sus datos
    for (Jugador *jugador : jugadores)
    {
        if (jugador->mano)
        {
            jugador->mano->hide();
            jugador->mano->deleteLater();
            jugador->mano = nullptr;
        }
        delete jugador;
    }
    jugadores.clear();
    mapJugadores.clear();

    // Limpiar cartaTriunfo si existe
    if (cartaTriunfo)
    {
        cartaTriunfo->hide();
        cartaTriunfo->deleteLater();
        cartaTriunfo = nullptr;
    }

    // Limpiar mazo si existe
    if (mazo)
    {
        mazo->hide();
        mazo->deleteLater();
        mazo = nullptr;
    }

    // Limpiar overlay
    if (overlay)
    {
        overlay->hide();
        overlay->deleteLater();
        overlay = nullptr;
        overlayMsg = nullptr;
    }
}

void EstadoPartida::actualizarPuntuacion(int equipo, int nuevoValor, std::function<void()> callback)
{
    QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();
    int width = screenSize.width(), height = screenSize.height();

    QLabel *label = (equipo == 1) ? puntosEquipo1Label : puntosEquipo2Label;
    int *valorPtr = (equipo == 1) ? &puntosEquipo1 : &puntosEquipo2;

    if (!label || nuevoValor == *valorPtr)
    {
        if (callback)
            callback();
        return;
    }

    int paso = (nuevoValor > *valorPtr) ? 1 : -1;
    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [=]() mutable
            {
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
        } });

    timer->start();
}

void EstadoPartida::onCartaDobleClick(Carta *carta)
{
    if (websocket)
    {
        QJsonObject msg;
        msg["accion"] = "jugar_carta";
        QJsonObject cartaJson;
        cartaJson["palo"] = carta->getPalo();
        bool ok;
        cartaJson["valor"] = carta->getValor().toInt(&ok);
        if (!ok)
            cartaJson["valor"] = 0;
        msg["carta"] = cartaJson;

        this->enviarMsg(
            msg);
    }
    qDebug() << "Jugar carta";
}

//////////////////////////////////////////////////////////////////////////////////////
/// Cola de eventos
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
/// Procesar mensajes
//////////////////////////////////////////////////////////////////////////////////////

void EstadoPartida::procesarCardPlayed(QJsonObject data, std::function<void()> callback)
{
    int jugadorId = data["jugador"].toObject()["id"].toInt();
    QJsonObject cartaJson = data["carta"].toObject();
    QString paloJugado = cartaJson["palo"].toString();
    QString valorJugado = QString::number(cartaJson["valor"].toInt());

    Jugador *jugador = mapJugadores.value(jugadorId, nullptr);
    if (!jugador || !jugador->mano)
        return;

    Carta *cartaParaAnimar = nullptr;
    QPoint origenAnimacion;

    // Yo juego la carta
    if (jugadorId == miId)
    {
        cartaParaAnimar = jugador->mano->extraerCarta(paloJugado, valorJugado);
        cartaParaAnimar->interactuable = false;
        if (cartaParaAnimar)
        {
            origenAnimacion = cartaParaAnimar->mapToGlobal(QPoint(0, 0));
            cartaParaAnimar->setParent(this);
        }
        else
        {
            cartaParaAnimar = new Carta(paloJugado, valorJugado, this);
            origenAnimacion = jugador->mano->mapToGlobal(
                QPoint(jugador->mano->width() / 2, jugador->mano->height() / 2));
        }
    }
    // Juega otro jugador
    else
    {
        Carta *cartaPlaceholder = jugador->mano->pop();
        if (cartaPlaceholder)
        {
            origenAnimacion = cartaPlaceholder->mapToGlobal(QPoint(0, 0));
            cartaPlaceholder->hide();
            cartaPlaceholder->deleteLater();
        }
        else
        {
            origenAnimacion = jugador->mano->mapToGlobal(
                QPoint(jugador->mano->width() / 2, jugador->mano->height() / 2));
        }
        cartaParaAnimar = new Carta(paloJugado, valorJugado, this);
    }

    // Si algo va mal
    if (!cartaParaAnimar)
    {
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

    QPropertyAnimation *anim = new QPropertyAnimation(cartaParaAnimar, "pos", this);
    anim->setDuration(500);
    anim->setStartValue(origenAnimacionLocal);
    anim->setEndValue(destinoAnimacionLocal);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QPropertyAnimation::finished, this, [=]()
            {
        jugador->mano->actualizarCartaJugada(paloJugado, valorJugado);
        cartaParaAnimar->hide();
        cartaParaAnimar->deleteLater();
        jugador->mano->dibujar();
        this->dibujarEstado();
        if (callback) callback(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void EstadoPartida::procesarCardDrawn(QJsonObject data, std::function<void()> callback)
{

    QJsonObject cartaJson = data.value("carta").toObject();
    QString palo = cartaJson["palo"].toString();
    QString valor = QString::number(cartaJson["valor"].toInt());

    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);

    // Actualizar cartas restantes en mazo central
    mazoRestante -= jugadores.size();
    if (mazoRestante <= 0)
        mazo->hide();

    // Actualizar manos
    for (Jugador *jugador : jugadores)
    {
        if (!jugador || !jugador->mano || jugador->mano->getNumCartas() >= 6)
            continue;

        Carta *carta = new Carta();
        if (jugador->id == miId)
            carta = new Carta(palo, valor);

        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(carta); // Parent effect to the card
        opacityEffect->setOpacity(0.0);                                            // Start completely transparent
        carta->setGraphicsEffect(opacityEffect);

        jugador->mano->agnadirCarta(carta);
        this->dibujarEstado();

        QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity", animationGroup);
        anim->setDuration(1000);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        ;
        anim->setEasingCurve(QEasingCurve::InCubic);
        animationGroup->addAnimation(anim);
    }

    connect(animationGroup, &QParallelAnimationGroup::finished, this, [=]()
            {
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
        } });

    if (animationGroup->animationCount() > 0)
    {
        animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else
    {
        delete animationGroup;
        this->dibujarEstado();
    }
}

void EstadoPartida::iniciarBotonesYEtiquetas()
{
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
    botonCantar = new BotonAccion("Cantar", [this]()
                                  { this->onCantar(); }, this);

    botonCambiarSiete = new BotonAccion("Cambiar siete", [this]()
                                        { this->onCambiarSiete(); }, this);

    QString textoPausa = enPausa ? "Anular pausa" : "Solicitar pausa";
    botonPausa = new BotonAccion(textoPausa, [this]()
                                 {
        if(enPausa) this->onAnularPausa();
        else this->onPausa(); }, this);

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
        "}");
    pausadosLabel->setAlignment(Qt::AlignCenter);
}

void EstadoPartida::procesarStartGame(QJsonObject data)
{
    this->setPartidaIniciada(true);
    ocultarOverlayEspera();

    this->iniciarBotonesYEtiquetas();
    this->actualizarEstado(data);
    this->dibujarEstado();

    QTimer::singleShot(250, this, [=]()
                       {
        this->dibujarEstado();
        enEjecucion = false;
        procesarSiguienteEvento(); });
}

void EstadoPartida::procesarTurnUpdate(QJsonObject data, std::function<void()> callback)
{
    QString mensaje = data.value("message").toString("Turno desconocido");
    this->mostrarMensaje(mensaje, callback);
}

void EstadoPartida::procesarEndGame(QJsonObject data, std::function<void()> /*callback*/)
{
    int ganador = data.value("ganador_equipo").toInt();
    int puntos1 = data.value("puntos_equipo_1").toInt();
    int puntos2 = data.value("puntos_equipo_2").toInt();

    QString mensaje;
    if (ganador == 0)
        mensaje = QString("¡Empate!\n%1 - %2").arg(puntos1).arg(puntos2);
    else
        mensaje = QString("¡Ha ganado el equipo %1!\n%2 - %3").arg(ganador).arg(puntos1).arg(puntos2);

    VentanaInfo *info = new VentanaInfo(mensaje, [this]()
                                        {
        if(this->onSalir) this->onSalir();
        this->deleteLater(); }, this);
}

void EstadoPartida::procesarPhaseUpdate(QJsonObject data, std::function<void()> callback)
{
    this->mostrarMensaje("Cambio a fase de arrastre", [=]()
                         {
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

        anim->start(QAbstractAnimation::DeleteWhenStopped); });
}

void EstadoPartida::procesarRoundResult(QJsonObject data, std::function<void()> callback)
{
    QJsonObject ganador = data.value("ganador").toObject();
    QString nombre = ganador.value("nombre").toString("Desconocido");
    int equipo = ganador.value("equipo").toInt(-1);

    QString mensaje = QString("¡%1 ganó la baza para el equipo %2!")
                          .arg(nombre)
                          .arg(equipo);

    int puntos1 = data.value("puntos_equipo_1").toInt();
    int puntos2 = data.value("puntos_equipo_2").toInt();

    // Mostrar overlay y luego actualizar puntuaciones
    this->mostrarMensaje(mensaje, [=]()
                         {
        int completados = 0;
        auto onDone = [this, callback]() {
            if (callback) callback();
        };

        if (puntos1 != puntosEquipo1)
            this->actualizarPuntuacion(1, puntos1, onDone);
        else
            this->actualizarPuntuacion(2, puntos2, onDone); });

    for (Jugador *j : jugadores)
    {
        if (!j || !j->mano)
            continue;

        Carta *copia = new Carta(j->mano->jugada()->getPalo(), j->mano->jugada()->getValor(), this);
        QPoint posGlobal = j->mano->jugada()->mapToGlobal(QPoint(0, 0));
        QPoint posLocal = this->mapFromGlobal(posGlobal);
        copia->setOrientacion(j->mano->getOrientacion());
        copia->move(posLocal);
        j->mano->ocultarCartaJugada();
        copia->show();
        copia->raise();

        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(copia);
        copia->setGraphicsEffect(effect);
        QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(1000);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QPropertyAnimation::finished, copia, [copia]()
                { copia->deleteLater(); });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void EstadoPartida::procesarPause(QJsonObject data, std::function<void()> callback)
{
    int jugadorId = data["jugador"].toObject()["id"].toInt();
    QString nombre = data["jugador"].toObject()["nombre"].toString();
    jugadoresPausa = data["num_solicitudes_pausa"].toInt();
    if (jugadorId == miId)
    {
        enPausa = true;
        if (botonPausa)
            botonPausa->setText("Anular pausa");
    }
    pausadosLabel->setText(QString("%1/%2").arg(jugadoresPausa).arg(jugadores.size()));
    QString msg = QString("%1 ha solicitado pausa").arg(nombre);
    mostrarMensaje(msg, callback);
}

void EstadoPartida::procesarResume(QJsonObject data, std::function<void()> callback)
{
    int jugadorId = data["jugador"].toObject()["id"].toInt();
    QString nombre = data["jugador"].toObject()["nombre"].toString();
    jugadoresPausa = data["num_solicitudes_pausa"].toInt();
    if (jugadorId == miId)
    {
        enPausa = false;
        if (botonPausa)
            botonPausa->setText("Solicitar pausa");
    }
    pausadosLabel->setText(QString("%1/%2").arg(jugadoresPausa).arg(jugadores.size()));
    QString msg = QString("%1 ha anulado su solicitud de pausa").arg(nombre);
    mostrarMensaje(msg, callback);
}

void EstadoPartida::procesarAllPause(QJsonObject data, std::function<void()> callback)
{
    QString mensaje = "La partida ha sido pausada";
    VentanaInfo *info = new VentanaInfo(mensaje, [this]()
                                        {
        if(this->onSalir) this->onSalir();
        this->deleteLater(); }, this);
}

void EstadoPartida::procesarError(QJsonObject data, std::function<void()> callback)
{
    auto *popup = new QLabel(data.value("message").toString("Error desconocido"), this);
    popup->setStyleSheet("QLabel {"
                         "background: rgba(50,50,50,220);"
                         "color: white; font-size: 18px;"
                         "padding: 6px 12px;"
                         "border-radius: 6px; border: 1px solid #888; }");
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

    connect(group, &QSequentialAnimationGroup::finished, this, [popup, callback]()
            {
        popup->deleteLater();
        if (callback) callback(); });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

//////////////////////////////////////////////////////////////////////////////////////
/// Overlay
//////////////////////////////////////////////////////////////////////////////////////

void EstadoPartida::mostrarMensaje(const QString &msg, std::function<void()> callback, int duracion)
{
    if (overlay)
    {
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

    QVBoxLayout *layout = new QVBoxLayout(overlay);
    layout->addWidget(overlayMsg, 0, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    overlay->setLayout(layout);
    overlay->show();

    // Animación
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(overlay);
    opacityEffect->setOpacity(0.0);
    overlay->setGraphicsEffect(opacityEffect);

    overlay->show();
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);
    QPropertyAnimation *fadeInAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeInAnim->setDuration(500);
    fadeInAnim->setStartValue(0.0);
    fadeInAnim->setEndValue(0.8);
    fadeInAnim->setEasingCurve(QEasingCurve::OutCubic);
    QPauseAnimation *holdAnim = new QPauseAnimation(duracion);
    QPropertyAnimation *fadeOutAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOutAnim->setDuration(500);
    fadeOutAnim->setStartValue(0.8);
    fadeOutAnim->setEndValue(0.0);
    fadeOutAnim->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(fadeInAnim);
    group->addAnimation(holdAnim);
    group->addAnimation(fadeOutAnim);

    connect(group, &QSequentialAnimationGroup::finished, this, [this, callback]()
            {
        if (overlay) {
            delete overlay;
            overlay = nullptr;
            overlayMsg = nullptr;
        }
        if(callback) callback(); });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

//////////////////////////////////////////////////////////////////////////////////////
/// Overlay espera
//////////////////////////////////////////////////////////////////////////////////////

void EstadoPartida::mostrarOverlayEspera(int jugadoresCola, int jugadoresMax)
{
    if (overlayEspera)
    {
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

    QVBoxLayout *layout = new QVBoxLayout(overlayEspera);
    layout->addWidget(labelEspera);
    overlayEspera->setLayout(layout);
    overlayEspera->show();
}

void EstadoPartida::actualizarOverlayEspera(int jugadoresCola, int jugadoresMax)
{
    if (labelEspera)
    {
        labelEspera->setText(QString("Esperando jugadores... (%1/%2)").arg(jugadoresCola).arg(jugadoresMax));
    }
}

void EstadoPartida::ocultarOverlayEspera()
{
    if (overlayEspera)
    {
        overlayEspera->deleteLater();
        overlayEspera = nullptr;
        labelEspera = nullptr;
    }
}
