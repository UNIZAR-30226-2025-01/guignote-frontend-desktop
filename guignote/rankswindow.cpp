// rankswindow.cpp
#include "rankswindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
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
#include <QPropertyAnimation>

// --- RangeBarWidget --------------------------------------------------------

class RangeBarWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int elo READ elo WRITE setElo NOTIFY eloChanged)
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
        // altura = sobresaliente(10) + barra(30) + sobresaliente(10) + texto(20) + padding(20)
        setFixedHeight(10 + 30 + 10 + 20 + 20);
    }

    int elo() const { return m_elo; }

    void setElo(int val) {
        if(m_elo == val) return;
        m_elo = val;
        emit eloChanged(val);
        update();
    }

signals:
    void eloChanged(int);

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
        int x = 0;
        for(int i = 0; i < n; ++i) {
            int nextX = (i + 1) * W / n;
            int segW = nextX - x;
            double fillW = 0.0;
            if(i == 0) {
                fillW = segW * qBound(0.0, double(m_elo) / m_thresholds[0], 1.0);
            } else if(i < n - 1) {
                int lo = m_thresholds[i - 1];
                int hi = m_thresholds[i];
                if(m_elo > lo) {
                    if(m_elo >= hi) fillW = segW;
                    else fillW = segW * double(m_elo - lo) / double(hi - lo);
                }
            } else {
                if(m_elo >= m_thresholds.back()) fillW = segW;
            }
            if(fillW > 0) {
                p.fillRect(x, barY, int(fillW), barH, QColor(m_colors[i]));
            }
            x = nextX;
        }

        // 3) separadores blancos sobresalientes
        p.setPen(QPen(Qt::white, 2));
        for (int i = 1; i < n; ++i) {
            int sepX = i * W / n;
            p.drawLine(sepX, barY - over, sepX, barY + barH + over);
        }

        // 4) dibujar umbrales debajo de cada separador
        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPointSize(14);
        p.setFont(f);
        int textY = barY + barH + over + 4;
        int textH = H - textY;
        for (int i = 1; i < n; ++i) {
            int x0 = i * W / n;
            int segW = W / n;
            QRect r(x0 - segW / 2, textY, segW, textH);
            p.drawText(r, Qt::AlignHCenter | Qt::AlignTop,
                       QString::number(m_thresholds[i-1]));
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

    QNetworkRequest req(QUrl("http://188.165.76.134:8000/usuarios/elo/"));
    req.setRawHeader("Auth", token.toUtf8());
    auto *reply = networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            auto doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject()) {
                int finalElo = doc.object().value("elo").toInt();

                QString rango_str;
                if (finalElo < 1200) rango_str = "Guiri";
                else if (finalElo < 1600) rango_str = "Casual";
                else if (finalElo < 2100) rango_str = "Parroquiano";
                else if (finalElo < 2700) rango_str = "Octogenario";
                else rango_str = "Leyenda del Imserso";

                eloLabel->setText(QString("Rango: %1").arg(rango_str));

                QPropertyAnimation* anim = new QPropertyAnimation(barWidget, "elo", this);
                anim->setStartValue(0);
                anim->setEndValue(finalElo);
                anim->setDuration(1500);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
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
    QVector<QString> rangos     = { "Guiri", "Casual", "Parroquiano", "Octogenario", "Leyenda" };


    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(40, 40, 40, 40);

    auto* content = new QWidget(this);
    content->setAttribute(Qt::WA_TranslucentBackground);
    content->setStyleSheet("background: transparent;");

    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(15);
    outerLayout->addWidget(content);

    // encabezado
    auto *headerLayout = new QHBoxLayout();
    auto *title = new QLabel("Tabla de Rangos", this);
    eloLabel = new QLabel("Tu rango: Guiri", this);
    eloLabel->setStyleSheet("background: transparent; color: #bbbbbb; font-size: 18px;");
    mainLayout->addWidget(eloLabel, 0, Qt::AlignLeft);
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
    iconsW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *iconsL = new QHBoxLayout(iconsW);
    iconsL->setContentsMargins(0,0,0,0);
    iconsL->setSpacing(0);
    int segW = 880 / colors.size();
    for (int i = 0; i < icons.size(); ++i) {
        QString iconName = icons[i];
        QString rangoName = rangos[i];

        QWidget* item = new QWidget(iconsW);
        item->setAttribute(Qt::WA_TranslucentBackground);
        item->setStyleSheet("background: transparent;");
        item->setFixedWidth(segW);

        QVBoxLayout* vbox = new QVBoxLayout(item);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->setSpacing(2);
        vbox->setAlignment(Qt::AlignHCenter);

        QLabel* iconLabel = new QLabel(item);
        iconLabel->setAttribute(Qt::WA_TranslucentBackground);
        iconLabel->setStyleSheet("background: transparent;");
        QPixmap pm(QString(":/icons/%1.png").arg(iconName));
        iconLabel->setPixmap(pm.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel* textLabel = new QLabel(rangoName, item);
        textLabel->setAttribute(Qt::WA_TranslucentBackground);
        textLabel->setStyleSheet("background: transparent; color: white; font-size: 16px;");
        textLabel->setAlignment(Qt::AlignCenter);

        vbox->addWidget(iconLabel);
        vbox->addWidget(textLabel);
        iconsL->addWidget(item);
    }
    mainLayout->addWidget(iconsW, 0, Qt::AlignHCenter);

    // barra de rangos
    int barWidth = 880;
    barWidget = new RangeBarWidget(colors, thresholds, this);
    barWidget->setFixedWidth(barWidth);
    mainLayout->addWidget(barWidget, 0, Qt::AlignHCenter);
    iconsW->setFixedWidth(barWidth);
    mainLayout->addWidget(iconsW, 0, Qt::AlignHCenter);

    // centrar verticalmente abajo
    mainLayout->addStretch();
}

#include "rankswindow.moc"
