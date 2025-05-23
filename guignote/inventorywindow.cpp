/**
 * @file inventorywindow.cpp
 * @brief Implementación de la clase InventoryWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Contiene la definición de la ventana de inventario, que permite
 * gestionar barajas y tapetes mediante una interfaz con tarjeta,
 * sombras y animaciones de transición.
 */


#include "inventorywindow.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainterPath>
#include <QStackedWidget>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QButtonGroup>
#include <QSettings>
#include <QStringList>
#include <QPixmap>
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <algorithm>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPainter>
#include <QEnterEvent>
#include <QListWidget>
#include <QFrame>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


/* ------------------------------------------------------------------ */
/*  ESTILO QSS COMÚN PARA LAS TARJETAS                                */
/* ------------------------------------------------------------------ */
static const char *tileQss = R"(
    CardTile {
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                                    stop:0 #37474F, stop:1 #263238);
        border: 1px solid #455A64;
        border-radius: 12px;
        color: #ECEFF1;
        font: 11px "Segoe UI Semibold";
    }
    CardTile:hover {
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                                    stop:0 #546E7A, stop:1 #37474F);
        border-color:#4dd0e1;
    }
    CardTile:checked {
           border: 2px solid #4dd0e1;   /* Borde destacado cuando está seleccionada */
    }
)";

/**
 * @brief Constructor de CardTile.
 * @param text Texto que mostrará la tarjeta.
 * @param pixmap Imagen opcional para mostrar en la tarjeta.
 * @param parent Widget padre.
 *
 * Configura tamaño fijo, estilo QSS, sombra, y permite marcado.
 */

class CardTile : public QPushButton
{
    Q_OBJECT
public:
    explicit CardTile(const QString &text,
                      const QPixmap &pixmap = QPixmap(),
                      QWidget *parent = nullptr)
        : QPushButton(text, parent)
        , m_pixmap(pixmap)
    {
        setFixedSize(150, 110);
        setStyleSheet(tileQss);
        setObjectName("CardTile");
        setFocusPolicy(Qt::NoFocus);
        setCheckable(true);
        connect(this, &QAbstractButton::toggled,
                this, [this](bool){
                    // Cuando cambie checked, forzamos un repaint
                    this->update();
                });
        shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(18);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0,0,0,160));
        setGraphicsEffect(shadow);
    }

protected:

    /**
 * @brief Constructor de CardTile.
 * @param text Texto que mostrará la tarjeta.
 * @param pixmap Imagen opcional para mostrar en la tarjeta.
 * @param parent Widget padre.
 *
 * Configura tamaño fijo, estilo QSS, sombra, y permite marcado.
 */

    void enterEvent(QEnterEvent *e) override {
        elevate(28, 6);
        QPushButton::enterEvent(e);
    }

    /**
 * @brief Restaura la sombra al salir el cursor de la tarjeta.
 * @param e Evento de salida del cursor.
 */

    void leaveEvent(QEvent *e) override {
        elevate(18, 4);
        QPushButton::leaveEvent(e);
    }

    /**
 * @brief Dibuja la tarjeta personalizada con esquina redondeada.
 * @param Evento de pintura (no utilizado directamente).
 *
 * Si tiene pixmap, lo dibuja centrado y añade borde de selección;
 * si no, dibuja texto centrado.
 */

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // 1) Fondo y clip
        const int radius = 12;
        QPainterPath path;
        path.addRoundedRect(rect(), radius, radius);
        p.setClipPath(path);
        QColor back = underMouse() ? QColor("#546E7A") : QColor("#37474F");
        p.fillPath(path, back);

        // 2) Si tenemos imagen, la dibujamos a tope y salimos:
        if (!m_pixmap.isNull()) {
            // Deja un margen mínimo
            QRect imgArea = rect().adjusted(5, 5, -5, -5);
            QPixmap scaled = m_pixmap.scaled(
                imgArea.size(),
                Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation);
            // Centrar el scaled dentro de imgArea
            QPoint pt(
                imgArea.x() + (imgArea.width()  - scaled.width())/2,
                imgArea.y() + (imgArea.height() - scaled.height())/2
                );
            p.drawPixmap(pt, scaled);

            // Borde de selección
            QPen pen(isChecked() ? QColor("#4dd0e1") : QColor("#455A64"),
                     isChecked() ? 3 : 1);
            p.setPen(pen);
            p.drawPath(path);
            return;  // ¡no dibujamos texto!
        }

        // 3) Si no hay imagen, dibujar borde y texto como antes
        QPen pen(isChecked() ? QColor("#4dd0e1") : QColor("#455A64"),
                 isChecked() ? 3 : 1);
        p.setPen(pen);
        p.drawPath(path);

        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 10, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, text());
    }


private:
    /**
 * @brief Ajusta dinámicamente el radio de desenfoque y el desplazamiento de la sombra.
 * @param blur Nuevo valor para blurRadius del efecto de sombra.
 * @param y Nuevo valor de offset vertical de la sombra.
 *
 * Crea animaciones que interpolan desde los valores actuales hasta los especificados,
 * para dar un efecto de elevación al pasar/retirar el cursor.
 */

    void elevate(qreal blur, qreal y)
    {
        auto animate = [this](const char *prop, const QVariant &v) {
            auto *a = new QPropertyAnimation(shadow, prop, this);
            a->setDuration(160);
            a->setStartValue(shadow->property(prop));
            a->setEndValue(v);
            a->setEasingCurve(QEasingCurve::OutCubic);
            a->start(QAbstractAnimation::DeleteWhenStopped);
        };
        animate("blurRadius", blur);
        animate("offset", QPointF(0, y));
    }

    QGraphicsDropShadowEffect *shadow;
    QPixmap m_pixmap;
};

/**
 * @brief Constructor de NavList.
 * @param parent Widget padre.
 *
 * Configura estilo QSS, sin marco, y prepara indicador animado.
 */

class NavList : public QListWidget
{
    Q_OBJECT
public:
    explicit NavList(QWidget *parent = nullptr) : QListWidget(parent)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setFrameShape(QFrame::NoFrame);
        setSpacing(4);

        setStyleSheet(R"(
            NavList {
                background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                                            stop:0 #263238, stop:1 #1e1e1e);
                color: #ECEFF1;
                font: 16px "Segoe UI";
            }
            NavList::item            { padding: 12px 24px; }
            NavList::item:selected,
            NavList::item:hover      { background: transparent; }
        )");

        indicator = new QWidget(this);
        indicator->setFixedWidth(4);
        indicator->setStyleSheet("background:#4dd0e1; border-radius:2px;");
        indicator->hide();

        connect(this, &QListWidget::currentRowChanged,
                this, &NavList::animateIndicator);
    }

private slots:
    /**
 * @brief Mueve el indicador a la fila seleccionada.
 * @param row Índice de la fila activa.
 *
 * Anima la propiedad geometry para deslizar el indicador.
 */

    void animateIndicator(int row)
    {
        if (row < 0) return;
        QRect r = visualItemRect(item(row));
        if (!indicator->isVisible()) {
            indicator->setGeometry(0, r.top(), 4, r.height());
            indicator->show();
        }
        auto *a = new QPropertyAnimation(indicator, "geometry", this);
        a->setDuration(250);
        a->setStartValue(indicator->geometry());
        a->setEndValue(QRect(0, r.top(), 4, r.height()));
        a->setEasingCurve(QEasingCurve::OutCubic);
        a->start(QAbstractAnimation::DeleteWhenStopped);
    }

private:
    QWidget *indicator;
};

/**
 * @brief Crea un efecto de opacidad inicializado en 1.0.
 * @param w Widget al que aplicar el efecto.
 * @return Puntero al QGraphicsOpacityEffect configurado.
 */

static QGraphicsOpacityEffect *makeFadeEffect(QWidget *w)
{
    auto *eff = new QGraphicsOpacityEffect(w);
    eff->setOpacity(1.0);
    eff->setEnabled(false);
    w->setGraphicsEffect(eff);
    return eff;
}

/**
 * @brief Anima la transición entre páginas de un QStackedWidget.
 * @param stack Pila de páginas.
 * @param nextIdx Índice de la página destino.
 * @param ms Duración de la animación en milisegundos.
 * @param dir Dirección del deslizamiento (+1 derecha→izq, -1 izquierdo→der).
 */

static void slidePages(QStackedWidget *stack, int nextIdx,
                       int ms = 300, int dir = +1)
{
    if (nextIdx == stack->currentIndex()) return;

    QWidget *fromW = stack->currentWidget();
    QWidget *toW   = stack->widget(nextIdx);

    const int w = stack->width();
    const int h = stack->height();

    QRect start(dir * w, 0, w, h);
    QRect end  (0,        0, w, h);

    toW->setGeometry(start);
    toW->show();
    toW->raise();

    auto move = [&](QWidget *w, const QRect &ini, const QRect &fin){
        auto *a = new QPropertyAnimation(w, "geometry", stack);
        a->setDuration(ms);
        a->setStartValue(ini);
        a->setEndValue(fin);
        a->setEasingCurve(QEasingCurve::InOutQuad);
        return a;
    };
    auto *aOut = move(fromW, QRect(0, 0, w, h),
                      QRect(-dir * w, 0, w, h));
    auto *aIn  = move(toW,   start, end);

    QObject::connect(aIn, &QPropertyAnimation::finished, stack, [=]{
        stack->setCurrentIndex(nextIdx);
        fromW->setGeometry(0, 0, w, h);
        toW  ->setGeometry(0, 0, w, h);
    });

    aOut->start(QAbstractAnimation::DeleteWhenStopped);
    aIn ->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief Constructor de InventoryWindow.
 * @param parent Widget padre.
 * @param usr Clave de usuario para cargar ajustes.
 *
 * Configura ventana sin marco, estilos, sombra y crea
 * la interfaz de selección de barajas y tapetes.
 */

InventoryWindow::InventoryWindow(QWidget *parent, QString usr) : QDialog(parent)
{
    /* ---------- Ventana base ---------- */
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(800, 600);

    QWidget *container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet("#container{background:#1e1e1e;border-radius:20px;}");
    container->setGeometry(rect());

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0,4);
    shadow->setColor(QColor(0,0,0,160));
    setGraphicsEffect(shadow);

    QVBoxLayout *root = new QVBoxLayout(container);
    root->setContentsMargins(0,0,0,0);

    /* ---------- Barra superior ---------- */
    QHBoxLayout *top = new QHBoxLayout;
    top->setContentsMargins(10,10,10,0);
    closeButton = new QPushButton;
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18,18));
    closeButton->setFixedSize(30,30);
    closeButton->setStyleSheet(
        "QPushButton{background:#444;border:none;border-radius:15px;}"
        "QPushButton:hover{background:#ff5c5c;}");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    top->addStretch();
    top->addWidget(closeButton);
    root->addLayout(top);

    /* ---------- Barra lateral + páginas ---------- */
    QHBoxLayout *main = new QHBoxLayout;
    main->setContentsMargins(10,0,10,10);

    sidebar = new NavList;
    auto *sideShadow = new QGraphicsDropShadowEffect(sidebar);
    sideShadow->setBlurRadius(30);
    sideShadow->setOffset(4,0);
    sideShadow->setColor(QColor(0,0,0,180));
    sidebar->setGraphicsEffect(sideShadow);
    sidebar->setFixedWidth(220);

    sidebar->addItem(new QListWidgetItem(qApp->style()->standardIcon(
                                             QStyle::SP_DialogYesButton), "Barajas"));
    sidebar->addItem(new QListWidgetItem(qApp->style()->standardIcon(
                                             QStyle::SP_FileDialogDetailedView), "Tapetes"));
    main->addWidget(sidebar);

    stackedWidget = new QStackedWidget;
    main->addWidget(stackedWidget, 1);
    root->addLayout(main);

    /* ====================  Página 1 : Barajas ==================== */
    m_userId = usr;
    deckPage = new QWidget;
    deckPage->setStyleSheet("background:#1e1e1e;");
    makeFadeEffect(deckPage);
    deckGroup = new QButtonGroup(this);
    deckGroup->setExclusive(true);

    // Esto **sólo** una vez:
    connect(deckGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &InventoryWindow::onDeckSelected,
            Qt::UniqueConnection);

    auto *vDeck = new QVBoxLayout(deckPage);
    vDeck->setAlignment(Qt::AlignTop);
    QLabel *lblDeck = new QLabel("Gestión de Barajas");
    lblDeck->setStyleSheet("color:#fff;font-size:24px;font-weight:bold;");
    vDeck->addWidget(lblDeck, 0, Qt::AlignHCenter);

    // 2) Añadir la página vacía al stack (sin tiles aún)
    stackedWidget->addWidget(deckPage);


    // 3) Primero: pedir el ID numérico para el username m_userId
    m_netMgr = new QNetworkAccessManager(this);

    // Slot específico para la primera respuesta
    QUrl urlId(QStringLiteral(
                   "http://188.165.76.134:8000/usuarios/usuarios/id/%1/")
                   .arg(m_userId));
    QNetworkRequest reqId(urlId);
    QNetworkReply *replyId = m_netMgr->get(reqId);
    connect(replyId, &QNetworkReply::finished, this, [this, replyId]() {
        onGetUserIdReply(replyId);
    });

    /* ====================  Página 2 : Tapetes ==================== */

    matPage = new QWidget;
    matPage->setStyleSheet("background:#1e1e1e;");
    makeFadeEffect(matPage);
    matGroup = new QButtonGroup(this);
    matGroup->setExclusive(true);

    // Esto **sólo** una vez:
    connect(matGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &InventoryWindow::onMatSelected,
            Qt::UniqueConnection);

    auto *vMat = new QVBoxLayout(matPage);
    vMat->setAlignment(Qt::AlignTop);
    QLabel *lblMat = new QLabel("Gestión de Tapetes");
    lblMat->setStyleSheet("color:#fff;font-size:24px;font-weight:bold;");
    vMat->addWidget(lblMat, 0, Qt::AlignHCenter);

    stackedWidget->addWidget(matPage);

    /* ---------- señales ---------- */
    sidebar->setCurrentRow(0);
    connect(sidebar, &QListWidget::currentRowChanged,
            this,    &InventoryWindow::onTabChanged);

}

// inventorywindow.cpp

void InventoryWindow::onDeckSelected(int skinId)
{
    // 1) Guardar localmente
    QSettings s("Grace Hopper", QString("Sota, Caballo y Rey_%1").arg(m_userId));
    s.setValue("selectedDeck", skinId);

    // 2) Enviar la petición al backend para equipar la skin
    QNetworkRequest req(QUrl(
        QStringLiteral("http://188.165.76.134:8000/usuarios/equip_skin/%1/")
            .arg(m_numericUserId)
        ));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body{{"skin_id", skinId}};
    QNetworkReply* reply = m_netMgr->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Skin equipada correctamente:" << reply->readAll();
        } else {
            qWarning() << "Error equipando skin:" << reply->errorString();
        }
        reply->deleteLater();
    });
}


/**
 * @brief Slot que responde al cambio de pestaña en la barra lateral.
 * @param row Índice de la pestaña seleccionada.
 *
 * Llama a slidePages para animar la transición.
 */

void InventoryWindow::onTabChanged(int row)
{
    slidePages(stackedWidget, row, 350, -1);  // slide de derecha a izquierda
}

void InventoryWindow::onGetUserIdReply(QNetworkReply *reply)
{
    // 1) Comprueba error y borra reply
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error obteniendo numeric ID:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    auto docId = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    m_numericUserId = docId.object().value("user_id").toInt(-1);
    if (m_numericUserId < 0) {
        qWarning() << "No vino user_id válido";
        return;
    }

    // 2) Obtener skin y tapete equipado
    QUrl urlEq(QStringLiteral(
                   "http://188.165.76.134:8000/usuarios/get_equipped_items/%1/")
                   .arg(m_numericUserId));
    QNetworkRequest reqEq(urlEq);
    QNetworkReply *replyEq = m_netMgr->get(reqEq);
    connect(replyEq, &QNetworkReply::finished, this, [this, replyEq]() {
        if (replyEq->error() == QNetworkReply::NoError) {
            auto doc = QJsonDocument::fromJson(replyEq->readAll());
            if (doc.isObject()) {
                auto o = doc.object();
                m_equippedSkinId = o.value("equipped_skin")
                                       .toObject()
                                       .value("id").toInt(-1);
                m_equippedMatId  = o.value("equipped_tapete")
                                      .toObject()
                                      .value("id").toInt(-1);
            }
        }
        replyEq->deleteLater();

        // 3) Pedir TODOS los ítems desbloqueados
        QUrl urlUnl(QStringLiteral(
                        "http://188.165.76.134:8000/usuarios/get_unlocked_items/%1/")
                        .arg(m_numericUserId));
        QNetworkReply *replyUnl = m_netMgr->get(QNetworkRequest(urlUnl));
        connect(replyUnl, &QNetworkReply::finished, this, [this, replyUnl]() {
            // 3.1) Comprobar errores
            if (replyUnl->error() != QNetworkReply::NoError) {
                qWarning() << "Error al obtener ítems desbloqueados:"
                           << replyUnl->errorString();
                replyUnl->deleteLater();
                return;
            }
            // 3.2) Leer y parsear JSON
            QByteArray data = replyUnl->readAll();
            replyUnl->deleteLater();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isObject()) {
                qWarning() << "Respuesta de unlocked_items no es un objeto";
                return;
            }
            QJsonObject obj = doc.object();
            // 3.3) Extraer y poblar
            populateDeckPage(obj.value("unlocked_skins").toArray());
            populateMatPage(obj.value("unlocked_tapetes").toArray());
        });
    });
}





void InventoryWindow::onUnlockedSkinsReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error al obtener skins:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    QJsonArray skins = doc.object().value("unlocked_skins").toArray();
    populateDeckPage(skins);
}

void InventoryWindow::onUnlockedMatsReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Error al obtener tapetes:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    auto doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    if (!doc.isObject()) return;
    QJsonArray mats = doc.object()
                          .value("unlocked_tapetes")
                          .toArray();
    populateMatPage(mats);
}


void InventoryWindow::populateDeckPage(const QJsonArray &skins)
{
    // 0) Limpiamos botones anteriores del grupo
    for (auto *btn : deckGroup->buttons())
        deckGroup->removeButton(btn);

    // 1) Eliminar el antiguo grid si existía
    auto *vlay = static_cast<QVBoxLayout*>(deckPage->layout());
    if (vlay->count() > 1) {
        QLayoutItem *li = vlay->takeAt(1);
        if (li) {
            if (auto *oldLayout = li->layout())
                delete oldLayout;
            delete li;
        }
    }

    // 2) Recoger IDs desbloqueados
    QSet<int> unlockedIds;
    for (auto v : skins)
        unlockedIds.insert(v.toObject().value("id").toInt());

    // 3) Mapa interno id → recurso
    static const QMap<int, QString> resourceMap = {
        {1, ":/tiles/base.png"},
        {2, ":/tiles/poker.png"},
        {3, ":/tiles/paint.png"}
    };

    // 4) Construir el nuevo grid
    auto *grid = new QGridLayout;
    grid->setSpacing(15);
    const int cols = 3;
    for (int idx = 1; idx <= 6; ++idx) {
        bool ok = unlockedIds.contains(idx) && resourceMap.contains(idx);
        QPixmap pix;
        if (ok) pix.load(resourceMap[idx]);

        CardTile *tile = new CardTile(QString("Baraja %1").arg(idx), pix);
        tile->setEnabled(ok);
        tile->setCheckable(ok);
        if (ok)
            deckGroup->addButton(tile, idx);

        int row = (idx - 1) / cols;
        int col = (idx - 1) % cols;
        grid->addWidget(tile, row, col);
    }

    // 5) Insertar el grid en la posición 1
    vlay->insertLayout(1, grid);

    // 6) Restaurar la skin equipada por el servidor (o fallback local)
    if (m_equippedSkinId > 0 && deckGroup->button(m_equippedSkinId)) {
        deckGroup->button(m_equippedSkinId)->setChecked(true);
    } else {
        QSettings settings("Grace Hopper",
                           QString("Sota, Caballo y Rey_%1").arg(m_userId));
        int stored = settings.value("selectedDeck", -1).toInt();
        if (auto *b = deckGroup->button(stored)) {
            b->setChecked(true);
        }
        else if (!skins.isEmpty()) {
            int first = skins.first().toObject().value("id").toInt();
            if (auto *b2 = deckGroup->button(first)) {
                b2->setChecked(true);
                settings.setValue("selectedDeck", first);
            }
        }
    }

}

void InventoryWindow::populateMatPage(const QJsonArray &mats)
{
    // 0) Limpiamos botones anteriores del grupo
    for (auto *btn : matGroup->buttons())
        matGroup->removeButton(btn);

    // 1) Eliminar el antiguo grid si existía
    auto *vlay = static_cast<QVBoxLayout*>(matPage->layout());
    if (vlay->count() > 1) {
        QLayoutItem *li = vlay->takeAt(1);
        if (li) {
            if (auto *oldLayout = li->layout())
                delete oldLayout;
            delete li;
        }
    }

    // 2) Recoger IDs desbloqueados
    QSet<int> unlockedIds;
    for (auto v : mats)
        unlockedIds.insert(v.toObject().value("id").toInt());

    // 3) Mapa interno id → recurso
    static const QMap<int, QString> matMap = {
        {1, ":/tiles/tapetebase.png"},
        {2, ":/tiles/rojo.png"},
        {3, ":/tiles/azul.png"},
        {4, ":/tiles/negro.png"}
    };

    // 4) Construir el nuevo grid (siempre 6 casillas: 2 filas x 3 cols)
    auto *grid = new QGridLayout;
    grid->setSpacing(15);
    const int cols    = 3;
    const int maxMats = 6;
    for (int idx = 1; idx <= maxMats; ++idx) {
        bool ok = unlockedIds.contains(idx) && matMap.contains(idx);
        QPixmap pix;
        if (ok) pix.load(matMap[idx]);

        CardTile *tile = new CardTile(QString("Tapete %1").arg(idx), pix);
        tile->setEnabled(ok);
        tile->setCheckable(ok);
        if (ok)
            matGroup->addButton(tile, idx);

        int row = (idx - 1) / cols;
        int col = (idx - 1) % cols;
        grid->addWidget(tile, row, col);
    }

    // 5) Insertar el grid en la posición 1
    vlay->insertLayout(1, grid);

    // 6) Restaurar el tapete equipado por servidor o fallback local
    if (m_equippedMatId > 0 && matGroup->button(m_equippedMatId)) {
        matGroup->button(m_equippedMatId)->setChecked(true);
    } else {
        QSettings settings("Grace Hopper",
                           QString("Sota, Caballo y Rey_%1").arg(m_userId));
        int stored = settings.value("selectedMat", -1).toInt();
        if (auto *b = matGroup->button(stored)) {
            b->setChecked(true);
        }
        else if (!mats.isEmpty()) {
            int first = mats.first().toObject().value("id").toInt();
            if (auto *b2 = matGroup->button(first)) {
                b2->setChecked(true);
                settings.setValue("selectedMat", first);
            }
        }
    }
}



void InventoryWindow::onMatSelected(int matId)
{
    // 1) guarda localmente
    QSettings s("Grace Hopper", QString("Sota, Caballo y Rey_%1").arg(m_userId));
    s.setValue("selectedMat", matId);

    // 2) llama al endpoint equip_tapete
    QNetworkRequest req(QUrl(
        QStringLiteral("http://188.165.76.134:8000/usuarios/equip_tapete/%1/")
            .arg(m_numericUserId)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body; body["tapete_id"] = matId;
    auto *reply = m_netMgr->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray raw = reply->readAll();
            qDebug() << "[DEBUG] Respuesta equip_tapete:" << QString::fromUtf8(raw);

            QJsonDocument doc = QJsonDocument::fromJson(raw);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                QJsonObject eq = obj.value("equipped_tapete").toObject();
                int equippedId = eq.value("id").toInt(-1);
                QString name = eq.value("name").toString();
                qDebug() << "[DEBUG] Tapete equipado → ID:" << equippedId << "Nombre:" << name;
            }
        } else {
            qWarning() << "[ERROR] Fallo al equipar tapete:" << reply->errorString();
        }

    });
}





/**
 * @brief Destructor de InventoryWindow.
 *
 * Limpia los recursos de la interfaz (implícito en Qt).
 */

InventoryWindow::~InventoryWindow() {}

#include "inventorywindow.moc"
