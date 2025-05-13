// rankswindow.cpp
#include "rankswindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPaintEvent>
#include <QRegularExpression>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>
#include <QSize>

// --- RangeBarWidget --------------------------------------------------------

class RangeBarWidget : public QWidget {
public:
    explicit RangeBarWidget(const QVector<QString>& colors,
                            const QVector<int>& thresholds,
                            QWidget* parent = nullptr)
        : QWidget(parent)
        , m_colors(colors)
        , m_thresholds(thresholds)
        , m_elo(0)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        // altura = sobresaliente(10) + barra(30) + sobresaliente(10) + texto(20)
        setFixedHeight(10 + 30 + 10 + 20);
    }

    void setElo(int elo) {
        m_elo = elo;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int W    = width();
        int H    = height();
        int over = 10;     // sobresale 10px arriba y abajo
        int barH = 30;     // altura de la franja
        int barY = over;
        int n    = m_colors.size();
        int segW = W / n;

        // 1) pintar fondo oscuro
        p.fillRect(0, barY, W, barH, QColor("#303030"));

        // 2) pintar overlay de colores según el elo
        for (int i = 0; i < n; ++i) {
            int x = i * segW;
            double fillW = 0.0;
            if (i == 0) {
                // primer segmento 0 – thresholds[0]
                fillW = segW * qBound(0.0, double(m_elo) / m_thresholds[0], 1.0);
            } else if (i < n - 1) {
                int lo = m_thresholds[i - 1];
                int hi = m_thresholds[i];
                if (m_elo > lo) {
                    if (m_elo >= hi) fillW = segW;
                    else fillW = segW * double(m_elo - lo) / double(hi - lo);
                }
            } else {
                // último segmento >= thresholds.back()
                if (m_elo >= m_thresholds.back()) fillW = segW;
            }
            if (fillW > 0) {
                p.fillRect(x, barY, int(fillW), barH, QColor(m_colors[i]));
            }
        }

        // 3) separadores blancos sobresalientes
        p.setPen(QPen(Qt::white, 2));
        for (int i = 1; i < n; ++i) {
            int x = i * segW;
            p.drawLine(x, barY - over, x, barY + barH + over);
        }

        // 4) dibujar umbrales debajo de cada separador
        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPointSize(14);
        p.setFont(f);
        int textY = barY + barH + over + 4;
        int textH = H - textY;
        for (int i = 1; i < n; ++i) {
            int x0 = i * segW;
            QRect r(x0 - segW / 2, textY, segW, textH);
            p.drawText(r, Qt::AlignHCenter | Qt::AlignTop,
                       QString::number(m_thresholds[i - 1]));
        }
    }

private:
    QVector<QString> m_colors;
    QVector<int>     m_thresholds;
    int              m_elo;
};


// --- RanksWindow implementation --------------------------------------------

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

RanksWindow::RanksWindow(const QString &userKey, QWidget *parent)
    : QDialog(parent)
    , userKey(userKey)
{
    networkManager = new QNetworkAccessManager(this);

    // diálogo sin marco y fondo oscuro
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(R"(
        QDialog {
            background-color: #171718;
            border-radius: 30px;
            padding: 20px;
        }
    )");
    setFixedSize(900, 650);

    setupUI();

    // sombra exterior
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(4, 4);
    setGraphicsEffect(shadow);

    // tras cargar la UI, obtenemos el ELO
    fetchElo();
}

QString RanksWindow::loadAuthToken() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "No se pudo abrir config:" << configPath;
        return {};
    }
    QString token;
    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        if (line.startsWith("token=")) {
            token = line.mid(QString("token=").length()).trimmed();
            break;
        }
    }
    return token;
}

void RanksWindow::fetchElo() {
    QString token = loadAuthToken();
    if (token.isEmpty()) return;

    QNetworkRequest req(QUrl("http://188.165.76.134:8000/elo/auth/"));
    req.setRawHeader("Authorization", "Bearer " + token.toUtf8());
    auto *reply = networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            auto doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject()) {
                int elo = doc.object().value("elo").toInt();
                barWidget->setElo(elo);
            }
        } else {
            qWarning() << "Error fetchElo:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void RanksWindow::setupUI() {
    // colores y umbrales
    QVector<QString> colors     = { "#fc03a5", "#045bcc", "#e0c200", "#3ea607", "#f0055b" };
    QVector<int>     thresholds = { 1200, 1600, 2100, 2700 };
    QVector<QString> icons      = { "flip-flops", "sofa", "beer", "80", "yogi" };

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20,20,20,20);
    mainLayout->setSpacing(15);

    // encabezado
    auto *headerLayout = new QHBoxLayout();
    auto *title = new QLabel("Tabla de Rangos", this);
    title->setStyleSheet("background: transparent; color: white; font: bold 28px;");
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    auto *closeBtn = new QPushButton(this);
    closeBtn->setIcon(QIcon(":/icons/cross.png"));
    closeBtn->setIconSize(QSize(22,22));
    closeBtn->setFixedSize(35,35);
    closeBtn->setStyleSheet(R"(
        QPushButton { background-color: #c2c2c3; border:none; border-radius:17px; }
        QPushButton:hover { background-color: #9b9b9b; }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    headerLayout->addWidget(closeBtn);
    mainLayout->addLayout(headerLayout);

    // centrar verticalmente
    mainLayout->addStretch();

    // iconos encima
    auto *iconsW = new QWidget(this);
    iconsW->setAttribute(Qt::WA_TranslucentBackground);
    iconsW->setStyleSheet("background: transparent;");
    iconsW->setFixedWidth(880);
    auto *iconsL = new QHBoxLayout(iconsW);
    iconsL->setContentsMargins(0,0,0,0);
    iconsL->setSpacing(0);
    int segW = 880 / colors.size();
    for (auto &name : icons) {
        auto *lbl = new QLabel(iconsW);
        lbl->setAttribute(Qt::WA_TranslucentBackground);
        lbl->setStyleSheet("background: transparent;");
        QPixmap pm(QString(":/icons/%1.png").arg(name));
        lbl->setPixmap(pm.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        lbl->setFixedWidth(segW);
        lbl->setAlignment(Qt::AlignCenter);
        iconsL->addWidget(lbl);
    }
    mainLayout->addWidget(iconsW, 0, Qt::AlignHCenter);

    // barra de rangos
    barWidget = new RangeBarWidget(colors, thresholds, this);
    barWidget->setFixedWidth(880);
    mainLayout->addWidget(barWidget, 0, Qt::AlignHCenter);

    // centrar verticalmente abajo
    mainLayout->addStretch();
}
