#include "myprofilewindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include "mainwindow.h"
#include <QDebug>
#include <QSettings>
#include "icon.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>

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
        "QPushButton { background-color: #c2c2c3; color: #171718; border-radius: 15px; "
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

QDialog* MyProfileWindow::createDialogLogOut(QWidget *parent, const QString &message) {
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog->setStyleSheet("QDialog { background-color: #2c2f32; border-radius: 5px; padding: 20px; border: none; }");

    // QGraphicsDropShadowEffect eliminado para no tener el borde ovalado en el texto

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QLabel *label = new QLabel(message, dialog);
    label->setWordWrap(true);
    label->setStyleSheet("color: white; font-size: 16px; border: none; background: transparent;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    // Crear los botones Sí y No con el mismo estilo que el botón original "OK"
    QPushButton *yesButton = new QPushButton("Sí", dialog);
    yesButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; color: #171718; border-radius: 15px; "
        "font-size: 20px; font-weight: bold; padding: 12px 25px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    yesButton->setFixedSize(100, 40);

    QPushButton *noButton = new QPushButton("No", dialog);
    noButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; color: #171718; border-radius: 15px; "
        "font-size: 20px; font-weight: bold; padding: 12px 25px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    noButton->setFixedSize(100, 40);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(yesButton);
    btnLayout->addWidget(noButton);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    // Conectar el botón "Sí" para realizar el log out y limpiar credenciales
    QObject::connect(yesButton, &QPushButton::clicked, [this, dialog]() {
        // Hacer LogOut
        dialog->close();
        QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
        // Elimina de forma controlada las credenciales y el token
        settings.remove("auth/user");
        settings.remove("auth/pass");
        settings.remove("auth/token");
        qDebug() << "Credenciales eliminadas correctamente desde QSettings.";

        QWidget *p = parentWidget();
        while (p) {
            p->close();               // Cierra este padre
            p = p->parentWidget();    // Avanza al siguiente en la jerarquía
        }
        this->close();

        // Creamos y mostramos la nueva MainWindow
        MainWindow *mw = new MainWindow();
        mw->show();
    });

    // Conectar el botón "No" solo para cerrar el diálogo
    QObject::connect(noButton, &QPushButton::clicked, [dialog]() {
        dialog->close();
    });

    dialog->adjustSize();
    dialog->move(parent->geometry().center() - dialog->rect().center());
    return dialog;
}


// Constructor: configura la ventana, la UI y carga los datos del backend.
MyProfileWindow::MyProfileWindow(const QString &userKey, QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(850, 680);

    setupUI();
    loadNameAndStats(userKey); // Se llama a la función que carga nombre, ELO y estadísticas.
}

// Configura la UI dividiéndola en secciones.
void MyProfileWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(createHeaderLayout());
    mainLayout->addLayout(createProfileLayout());
    mainLayout->addLayout(createBottomLayout());

    setLayout(mainLayout);
}

// Crea el layout del encabezado.
QHBoxLayout* MyProfileWindow::createHeaderLayout() {
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Perfil", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    return headerLayout;
}

// Crea el layout central: foto de perfil y etiquetas para nombre/ELO y estadísticas.
QVBoxLayout* MyProfileWindow::createProfileLayout() {
    QVBoxLayout *profileLayout = new QVBoxLayout();
    profileLayout->setAlignment(Qt::AlignCenter);
    profileLayout->addStretch();

    int pfpSize = 200;
    QString imagePath = ":/icons/profile.png";
    QPixmap circularImage = createCircularImage(imagePath, pfpSize);

    fotoPerfil = new Icon();
    fotoPerfil->setHoverEnabled(false);
    fotoPerfil->setPixmap(circularImage);
    fotoPerfil->setFixedSize(pfpSize, pfpSize);
    connect(fotoPerfil, &Icon::clicked, [=]() {
        qDebug() << "Foto de perfil clickada";
    });
    profileLayout->addWidget(fotoPerfil, 0, Qt::AlignCenter);

    // userLabel se actualizará con nombre y ELO desde el backend.
    userLabel = new QLabel("<span style='font-size: 24px; font-weight: bold; color: white;'>Usuario(0)</span><br>"
                           "<span style='font-size: 20px; font-weight: normal; color: white;'>Rango</span>", this);
    userLabel->setAlignment(Qt::AlignCenter);
    userLabel->setTextFormat(Qt::RichText);
    userLabel->setStyleSheet("color: white;");
    profileLayout->addWidget(userLabel);

    // statsLabel para estadísticas adicionales.
    statsLabel = new QLabel("Victorias: 0\nDerrotas: 0\nRacha: 0\nMejor Racha: 0\nPartidas: 0\n% Victorias: 0.0%\n% Derrotas: 0.0%", this);
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setStyleSheet("color: white; font-size: 20px;");
    profileLayout->addWidget(statsLabel);

    profileLayout->addStretch();
    return profileLayout;
}

// Crea el layout inferior con el botón de Log Out.
// Crea el layout inferior con el botón de Log Out.
QHBoxLayout* MyProfileWindow::createBottomLayout() {
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    logOutButton = new QPushButton("Log Out", this);
    logOutButton->setStyleSheet(
        "QPushButton { background-color: red; color: white; font-size: 18px; padding: 10px; border-radius: 5px; "
        "border: 2px solid #8B0000; }"
        "QPushButton:hover { background-color: #cc0000; }"
        "QPushButton:pressed { background-color: #8B0000; }"
        );
    logOutButton->setFixedSize(200, 50);
    bottomLayout->addStretch();
    bottomLayout->addWidget(logOutButton, 0, Qt::AlignRight);

    // Conexión para el botón Log Out
    connect(logOutButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "Botón Log Out presionado";
        createDialogLogOut(this, "¿Cerrar Sesion?")->show();
    });

    return bottomLayout;
}

// Convierte una imagen en un QPixmap circular.
QPixmap MyProfileWindow::createCircularImage(const QString &imagePath, int size) {
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        qDebug() << "Error: No se pudo cargar la imagen desde " << imagePath;
        return QPixmap();
    }
    pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circularPixmap(size, size);
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, size, size, pixmap);
    return circularPixmap;
}

// Extrae el token de autenticación desde el archivo de configuración.
QString MyProfileWindow::loadAuthToken(const QString &userKey) {
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

// Conecta con el backend para obtener el nombre, el ELO y las estadísticas, y actualiza userLabel y statsLabel.
void MyProfileWindow::loadNameAndStats(const QString &userKey) {
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        createDialog(this, "No se encontró el token de autenticación.")->show();
        return;
    }
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://188.165.76.134:8000/usuarios/estadisticas/"));
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            reply->deleteLater();
            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                // Actualiza userLabel con nombre y ELO.
                QString nombre = obj.value("nombre").toString();
                int elo = obj.value("ELO").toInt();
                QString updatedText = QString(
                                          "<span style='font-size: 24px; font-weight: bold; color: white;'>%1 (%2)</span><br>"
                                          "<span style='font-size: 20px; font-weight: normal; color: white;'>Rango</span>"
                                          ).arg(nombre).arg(elo);
                userLabel->setText(updatedText);

                // Extrae y actualiza las estadísticas.
                int victorias = obj.value("victorias").toInt();
                int derrotas = obj.value("derrotas").toInt();
                int racha = obj.value("racha_victorias").toInt();
                int mayorRacha = obj.value("mayor_racha_victorias").toInt();
                int totalPartidas = obj.value("total_partidas").toInt();
                double porcentajeVictorias = obj.value("porcentaje_victorias").toDouble();
                double porcentajeDerrotas = obj.value("porcentaje_derrotas").toDouble();
                QString statsText = QString("Victorias: %1\nDerrotas: %2\nRacha: %3\nMejor Racha: %4\nPartidas: %5\n"
                                            "%% Victorias: %6%\n%% Derrotas: %7%")
                                        .arg(victorias)
                                        .arg(derrotas)
                                        .arg(racha)
                                        .arg(mayorRacha)
                                        .arg(totalPartidas)
                                        .arg(porcentajeVictorias, 0, 'f', 1)
                                        .arg(porcentajeDerrotas, 0, 'f', 1);
                statsLabel->setText(statsText);
            }
        } else {
            createDialog(this, "Error al cargar el perfil de usuario.")->show();
        }
        reply->deleteLater();
    });
}
