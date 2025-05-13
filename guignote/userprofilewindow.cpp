/**
 * @file myprofilewindow.cpp
 * @brief Implementación de la clase MyProfileWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Define la ventana de perfil de usuario, donde se muestran y pueden modificar
 * los datos del usuario (nombre, ELO, estadísticas), cambiar la foto de perfil,
 * cerrar sesión o eliminar la cuenta. Gestiona la comunicación con el servidor
 * para cargar y actualizar dicha información.
 */

#include "userprofilewindow.h"
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
#include <QFileDialog>
#include <QHttpMultiPart>
#include <QHttpPart>


/**
 * @brief Crea un diálogo modal genérico con un mensaje y un botón OK.
 * @param parent Widget padre para centrar el diálogo.
 * @param message Texto a mostrar en el diálogo.
 * @param exitApp Si es true, al pulsar OK se cierra toda la aplicación.
 * @return Puntero al QDialog creado.
 */
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

/**
 * @brief Constructor de MyProfileWindow.
 * @param userKey Clave o identificador del usuario para cargar sus datos.
 * @param parent Widget padre, por defecto nullptr.
 */
UserProfileWindow::UserProfileWindow( QWidget *parent, const QString &userKey, QString friendId ) : QDialog(parent), m_userKey(userKey) {
    this->friendId = friendId;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(850, 680);

    setupUI();
    loadNameAndStats(userKey); // Se llama a la función que carga nombre, ELO y estadísticas.
    m_userKey = userKey;
}

/**
 * @brief Configura la interfaz de la ventana de perfil.
 *
 * Divide la ventana en secciones (encabezado, perfil y pie) y crea
 * los layouts con todos los widgets necesarios.
 */
void UserProfileWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(createHeaderLayout());
    mainLayout->addLayout(createProfileLayout());

    setLayout(mainLayout);
}

/**
 * @brief Crea el layout del encabezado con el título "Perfil" y el botón de cierre.
 * @return Un puntero a QHBoxLayout ya configurado.
 */
QHBoxLayout* UserProfileWindow::createHeaderLayout() {
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Perfil Amigo", this);
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

/**
 * @brief Crea el layout central que contiene la foto de perfil,
 *        el nombre/ELO y las estadísticas del usuario.
 * @return Un puntero a QVBoxLayout ya configurado.
 */
QVBoxLayout* UserProfileWindow::createProfileLayout() {
    QVBoxLayout *profileLayout = new QVBoxLayout();
    profileLayout->setAlignment(Qt::AlignCenter);
    profileLayout->addStretch();

    int pfpSize = 200;
    QString imagePath = ":/icons/profile.png";
    QPixmap circularImage = createCircularImage(imagePath, pfpSize);

    fotoPerfil = new Icon(this);
    fotoPerfil->setHoverEnabled(false);
    fotoPerfil->setPixmapImg(circularImage, pfpSize, pfpSize);
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



/**
 * @brief Convierte un QPixmap rectangular en uno circular.
 * @param src Imagen original a recortar.
 * @param size Diámetro deseado para la imagen circular.
 * @return QPixmap resultante con recorte circular.
 */
QPixmap UserProfileWindow::createCircularImage(const QPixmap &src, int size) {
    // Escalamos a un cuadrado de “size × size” y recortamos el exceso
    QPixmap scaled = src.scaled(size, size,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);
    QPixmap circular(size, size);
    circular.fill(Qt::transparent);

    QPainter painter(&circular);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, size, size, scaled);

    return circular;
}

/**
 * @brief Extrae el token de autenticación almacenado en QSettings.
 * @param userKey Clave del usuario cuyo token se desea recuperar.
 * @return Token como QString, o cadena vacía si no se encuentra.
 */
QString UserProfileWindow::loadAuthToken(const QString &userKey) {
    // Usa la misma organización y nombre de aplicación/grupo que en LoginWindow
    QSettings settings("Grace Hopper", QString("Sota, Caballo y Rey_%1").arg(userKey));

    // Lee el valor usando la clave correcta
    QString token = settings.value("auth/token").toString();

    if (token.isEmpty()) {
        qWarning() << "No se encontró el token para userKey:" << userKey << "en la configuración.";
        // Mantén o ajusta tu diálogo de error si es necesario
        createDialog(this, "No se encontró el token en la configuración.")->show();
        return ""; // Devuelve vacío si no se encuentra
    }

    qDebug() << "Token cargado para userKey:" << userKey;
    return token;
}

/**
 * @brief Solicita al servidor nombre, ELO y estadísticas del usuario,
 *        actualiza las etiquetas userLabel y statsLabel, y descarga la foto de perfil.
 * @param userKey Clave del usuario para la petición.
 */
void UserProfileWindow::loadNameAndStats(const QString &userKey) {
    QString token = loadAuthToken(userKey);
    if (token.isEmpty()) {
        createDialog(this, "No se encontró el token de autenticación.")->show();
        return;
    }
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString url = QString("http://188.165.76.134:8000/usuarios/estadisticas/%1").arg(friendId);
    QNetworkRequest request{QUrl(url)};
    request.setRawHeader("Auth", token.toUtf8());
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            createDialog(this, "Su sesión ha caducado, por favor, vuelva a iniciar sesión.", true)->show();
            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                // Actualiza userLabel con nombre y ELO.
                QString nombre = obj.value("nombre").toString();
                int elo = obj.value("elo").toInt();
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

                QString imageUrl = obj.value("imagen").toString();
                qDebug() << "URL de la imagen de perfil:" << imageUrl;
                QUrl url(imageUrl);
                QNetworkRequest imgRequest(url);
                QNetworkReply *imgReply = manager->get(imgRequest);
                connect(imgReply, &QNetworkReply::finished, this, [this, imgReply]() {
                    if (imgReply->error() == QNetworkReply::NoError) {
                        QByteArray imgData = imgReply->readAll();
                        qDebug() << "[ImgDownload] Tamaño de imgData:" << imgData.size();

                        QPixmap pixmap;
                        bool loaded = pixmap.loadFromData(imgData);
                        qDebug() << "[ImgDownload] loadFromData() =" << loaded
                                 << ", pixmap.isNull() =" << pixmap.isNull()
                                 << ", tamaño pixmap =" << pixmap.size();

                        if (!pixmap.loadFromData(imgData)) {
                            qDebug() << "❌ No se pudo cargar pixmap de los datos. Tamaño de imgData:" << imgData.size();
                            imgReply->deleteLater();
                            return;
                        }
                        if (pixmap.isNull()) {
                            qDebug() << "❌ El pixmap sigue siendo nulo después de loadFromData.";
                            imgReply->deleteLater();
                            return;
                        }

                        int diam = fotoPerfil->width();
                        QPixmap circular = createCircularImage(pixmap, diam);
                        fotoPerfil->setPixmapImg(circular, diam, diam);
                    } else {
                        qDebug() << "Error al descargar la imagen:" << imgReply->errorString();
                    }
                    imgReply->deleteLater();
                });
            }
        } else {
            createDialog(this, "Error al cargar el perfil de usuario.")->show();
        }
        reply->deleteLater();
    });
}
