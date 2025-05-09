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
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPainter>
#include <QEnterEvent>
#include <QListWidget>
#include <QFrame>

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

/* ------------------------------------------------------------------ */
/*  TARJETA (BOTÓN) CON ESQUINAS REDONDEADAS Y SOMBRA                 */
/* ------------------------------------------------------------------ */
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
    /* --- animamos solo blur y offset; la geometría no cambia --- */
    void enterEvent(QEnterEvent *e) override {
        elevate(28, 6);
        QPushButton::enterEvent(e);
    }
    void leaveEvent(QEvent *e) override {
        elevate(18, 4);
        QPushButton::leaveEvent(e);
    }

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

/* ------------------------------------------------------------------ */
/*  LISTA LATERAL (NavList)                                           */
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/*  FUNCIÓN AUXILIAR DE OPACIDAD (sin cambios)                         */
/* ------------------------------------------------------------------ */
static QGraphicsOpacityEffect *makeFadeEffect(QWidget *w)
{
    auto *eff = new QGraphicsOpacityEffect(w);
    eff->setOpacity(1.0);
    eff->setEnabled(false);
    w->setGraphicsEffect(eff);
    return eff;
}

/* ------------------------------------------------------------------ */
/*  TRANSICIÓN ENTRE PÁGINAS (sin cambios)                            */
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/*  CONSTRUCTOR / DESTRUCTOR DE INVENTORYWINDOW                       */
/* ------------------------------------------------------------------ */
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
    QWidget *deckPage = new QWidget;
    deckPage->setStyleSheet("background:#1e1e1e;");
    makeFadeEffect(deckPage);
    deckGroup = new QButtonGroup(this);
    deckGroup->setExclusive(true);
    QVBoxLayout *vDeck = new QVBoxLayout(deckPage);
    vDeck->setAlignment(Qt::AlignTop);
    QLabel *lblDeck = new QLabel("Gestión de Barajas");
    lblDeck->setStyleSheet("color:#fff;font-size:24px;font-weight:bold;");
    vDeck->addWidget(lblDeck, 0, Qt::AlignHCenter);

    QStringList deckImages = {
        ":/tiles/base.png",
        ":/tiles/poker.png",
        "", "", "", ""
    };
    QGridLayout *gridDeck = new QGridLayout;
    gridDeck->setSpacing(15);
    for (int i = 0; i < 6; ++i) {
        QPixmap pix(deckImages[i]);             // carga directa desde recurso
        CardTile *tile = new CardTile(
            QString("Baraja %1").arg(i+1), pix
            );
        bool has = !pix.isNull();
        tile->setCheckable(has);
        tile->setEnabled(has);
        if (has)
            deckGroup->addButton(tile, i);
        gridDeck->addWidget(tile, i/3, i%3);
    }
    vDeck->addLayout(gridDeck);

    // ¡AHORA! Conecta deckGroup para guardar la selección
    connect(deckGroup, &QButtonGroup::idClicked,
            this, [=](int id){
        QString config = QString("Sota, Caballo y Rey_%1").arg(usr);
        QSettings settings("Grace Hopper", config);
        settings.setValue("selectedDeck", id);
            });
    stackedWidget->addWidget(deckPage);

    /* ====================  Página 2 : Tapetes ==================== */
    QWidget *matPage = new QWidget;
    matPage->setStyleSheet("background:#1e1e1e;");
    makeFadeEffect(matPage);
    matGroup = new QButtonGroup(this);
    matGroup->setExclusive(true);
    QVBoxLayout *vMat = new QVBoxLayout(matPage);
    vMat->setAlignment(Qt::AlignTop);
    QLabel *lblMat = new QLabel("Gestión de Tapetes");
    lblMat->setStyleSheet("color:#fff;font-size:24px;font-weight:bold;");
    vMat->addWidget(lblMat, 0, Qt::AlignHCenter);

    QStringList matImages = {
        ":/tiles/tapetebase.png", // tapete 1
        "", "", "", "", ""
    };
    QGridLayout *gridMat = new QGridLayout;
    gridMat->setSpacing(15);
    for (int i = 0; i < 6; ++i) {
        // Carga directa desde recurso (queda null si ruta vacía)
        QPixmap pix(matImages[i]);

        // Construye la tarjeta con su imagen (si la hay)
        CardTile *tile = new CardTile(
            QString("Tapete %1").arg(i+1),
            pix
            );

        // Solo habilitar y checkable si la pixmap no está vacía
        bool has = !pix.isNull();
        tile->setCheckable(has);
        tile->setEnabled(has);

        // Si tiene imagen, lo añadimos al grupo para selección
        if (has)
            matGroup->addButton(tile, i);

        gridMat->addWidget(tile, i/3, i%3);
    }
    vMat->addLayout(gridMat);

    // Conecta la selección para persistir en QSettings
    connect(matGroup, &QButtonGroup::idClicked,
            this, [=](int id){
                QSettings().setValue("selectedMat", id);
            });
    stackedWidget->addWidget(matPage);

    /* ---------- señales ---------- */
    sidebar->setCurrentRow(0);
    connect(sidebar, &QListWidget::currentRowChanged,
            this,    &InventoryWindow::onTabChanged);

    QSettings s;
    int selDeck = s.value("selectedDeck", 0).toInt();
    if (auto *b = deckGroup->button(selDeck)) b->setChecked(true);

    int selMat = s.value("selectedMat", 0).toInt();
    if (auto *b2 = matGroup->button(selMat)) b2->setChecked(true);
}


void InventoryWindow::onTabChanged(int row)
{
    slidePages(stackedWidget, row, 350, -1);  // slide de derecha a izquierda
}

InventoryWindow::~InventoryWindow() {}

#include "inventorywindow.moc"
