/**
 * @file rejoinwindow.cpp
 * @brief Implementación de la clase RejoinWindow.
 *
 * Este archivo forma parte del Proyecto de Software 2024/2025
 * del Grado en Ingeniería Informática en la Universidad de Zaragoza.
 *
 * Esta clase muestra un diálogo con una lista de salas reconectables (recibidas como JSON),
 * permitiendo al usuario volver a unirse a una partida via WebSocket. Gestiona la creación de la UI,
 * el manejo de mensajes entrantes y la reconexión usando el token de autenticación.
 */


#include "rejoinwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWebSocket>
#include "menuwindow.h"
#include <QApplication>

/**
 * @brief Constructor de RejoinWindow.
 * @param jsonArray Array JSON con la información de las salas.
 * @param fondo Identificador de recurso de fondo.
 * @param userKey Clave del usuario para extraer el token.
 * @param usr Nombre del usuario actual.
 * @param parent Widget padre, puede ser nullptr.
 *
 * Configura el diálogo (sin bordes y con estilo), carga el token,
 * fija tamaño y estilo, y llama a setupUI() para construir la interfaz.
 */
RejoinWindow::RejoinWindow(QJsonArray salas, QJsonArray salasPausadas, int fondo, QString &userKey, QString usr, QWidget *parent)
    : QDialog(parent), salas(salas), salasPausadas(salasPausadas) {
    this->fondo = fondo;
    this->usr = usr;
    this->userKey = userKey;

    if (salas.isEmpty()) {
        qDebug() << "No hay salas reconectables";
    } else {
        qDebug() << "Número de salas reconectables:" << salas.size();
    }

    if (salasPausadas.isEmpty()) {
        qDebug() << "No hay salas pausadas";
    } else {
        qDebug() << "Número de salas pausadas:" << salasPausadas.size();
    }

    token = loadAuthToken(userKey);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(850, 680);
    setupUI();
}

/**
 * @brief Construye la interfaz de usuario.
 *
 * Crea el layout principal vertical, ajusta márgenes y espaciados,
 * añade el header y llama a populateSalas() para generar la lista.
 */
void RejoinWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(createHeaderLayout());
    mainLayout->addLayout(checkboxLayout());
    salasLayout = new QVBoxLayout();
    populateSalas();
    mainLayout->addLayout(salasLayout);

    setLayout(mainLayout);
}

/**
 * @brief Crea y devuelve el layout del encabezado.
 * @return Puntero al QHBoxLayout que contiene el título y el botón de cerrar.
 *
 * Configura el QLabel con el título y el QPushButton de cierre,
 * con estilo y conexiones adecuadas.
 */
QHBoxLayout* RejoinWindow::createHeaderLayout() {
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Título
    titleLabel = new QLabel("Reconexión", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    // Botón de cerrar
    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 15px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    // Agregar los widgets al layout
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);

    return headerLayout;
}

QHBoxLayout* RejoinWindow::checkboxLayout(){
    QHBoxLayout *layout = new QHBoxLayout();

    // Crear el QCheckBox
    pausadas = new QCheckBox("Salas Pausadas", this);
    pausadas->setStyleSheet(R"(
        QCheckBox { color: #ffffff; font-size: 14px; }
        QCheckBox::indicator { width: 16px; height: 16px; }
        QCheckBox::indicator:unchecked {
            background-color: #c2c2c3;
            border: 1px solid #545454;
        }
        QCheckBox::indicator:checked {
            background-color: #c2c2c3;
            border: 1px solid #545454;
            image: url(:/icons/cross.png);
        }
    )");

    // Conectar el cambio de estado del checkbox a la variable 'pausadas'
    connect(pausadas, &QCheckBox::toggled, this, [this](bool checked) {
        pausadasB = checked;  // Asigna el estado del checkbox a la variable 'pausadas'
        populateSalas();
        qDebug() << "Estado de pausadas:" << pausadasB;
    });

    layout->addWidget(pausadas);  // Agregar el QCheckBox debajo del título
    layout->setAlignment(pausadas, Qt::AlignCenter);

    return layout;
}


/**
 * @brief Rellena el diálogo con las salas reconectables.
 *
 * Para cada objeto en `salas` crea un contenedor con nombre,
 * ocupación y un botón "Entrar" que llama a rejoin().
 */
void RejoinWindow::populateSalas() {
    // Limpiar solo el layout que contiene las salas
    QLayoutItem *item;
    while ((item = salasLayout->takeAt(0)) != nullptr) {
        delete item->widget();  // Eliminar solo los widgets de las salas
        delete item;            // Eliminar el item del layout
    }

    // Seleccionamos el JsonArray basado en el valor de partidasB
    QJsonArray arraySalas = pausadasB ? salasPausadas : salas;

    for (const QJsonValue &val : arraySalas) {

        // Dentro del bucle de populateSalas(), justo al inicio:
        QJsonObject obj = val.toObject();

        if (!val.isObject()) continue;

        QJsonValue idVal = obj.value("id");
        QString idStr;
        if (idVal.isString()) {
            idStr = idVal.toString();
        } else {
            // Si viene como número, lo convertimos a entero y luego a cadena
            idStr = QString::number(idVal.toInt());
        }
        QString nombre       = obj.value("nombre").toString();
        int     capacidad    = obj.value("capacidad").toInt();
        int     numJugadores = obj.value("num_jugadores").toInt();

        // ——— Creamos un contenedor con fondo más claro ———
        QWidget *container = new QWidget(this);
        container->setProperty("isSalaWidget", true);  // Marcamos el widget como relacionado con las salas
        container->setStyleSheet(
            "background-color: #232326;"   // ligeramente más claro que #171718
            "border-radius: 10px;"
            "padding: 10px;"
            );
        // Le damos un layout horizontal
        QHBoxLayout *row = new QHBoxLayout(container);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(15);

        // ——— Label con nombre y ocupación ———
        QLabel *label = new QLabel(
            QString("%1  —  %2/%3")
                .arg(nombre)
                .arg(numJugadores)
                .arg(capacidad),
            container
            );
        label->setStyleSheet("color: white; font-size: 18px;");

        // ——— Botón verde ———
        QPushButton *btn = new QPushButton("Entrar", container);
        btn->setStyleSheet(
            "QPushButton {"
            "  background-color: #28a745;"
            "  color: white;"
            "  border-radius: 5px;"
            "  padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #218838;"
            "}"
            );
        connect(btn, &QPushButton::clicked, this, [this, idStr, capacidad]() {
            qDebug() << "Sala" << idStr << "pulsada";
            rejoin(idStr, capacidad);
        });

        // ——— Montamos la fila dentro del contenedor ———
        row->addWidget(label);
        row->addStretch();
        row->addWidget(btn);
        container->setLayout(row);

        // ——— Añadimos un pequeño margen vertical entre contenedores ———
        salasLayout->addWidget(container);
        salasLayout->setSpacing(10);
    }
}


/**
 * @brief Extrae el token de autenticación desde el archivo de configuración.
 * @param userKey Clave del usuario usada para formar la ruta del .conf.
 * @return Cadena con el token, o vacía si ocurre un error.
 *
 * Abre el fichero `.conf` asociado, busca la línea `token=...`
 * y devuelve su valor.
 */
QString RejoinWindow::loadAuthToken(const QString &userKey) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);

    qDebug() << "userKey: " << userKey;

    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Pon un qDebug() o algo para saber que falló
        qDebug() << "No se pudo abrir el archivo de configuración en MenuWindow." << configFile.fileName();
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
    return token;
}

/**
 * @brief Inicia la reconexión a la sala seleccionada.
 * @param idPart Identificador de la sala (string).
 *
 * Crea y configura un QWebSocket con el token y el id_partida,
 * conecta las señales y abre la conexión al servidor.
 */
void RejoinWindow::rejoin(QString idPart, int cap) {
    QString url = QString("ws://188.165.76.134:8000/ws/partida/?token=%1&id_partida=%2&capacidad=%3")
                      .arg(token)
                      .arg(idPart)
                      .arg(cap);
    qDebug() << "Conectando a:" << url;

    // Creamos la nueva ventana (EstadoPartida o GameWindow)
    QString usrKey = userKey;
    EstadoPartida *gameWindow = new EstadoPartida(usr, userKey, url, /** tapete */ 1, /** skin */1, [usrKey]() {
        auto* menu = new MenuWindow(usrKey);
        menu->showFullScreen();
    });

    gameWindow->setAttribute(Qt::WA_DeleteOnClose);
    gameWindow->setWindowFlag(Qt::Window); // Asegura que sea una ventana independiente

    // Le damos la misma geometría que el menú actual
    gameWindow->showFullScreen();

    QTimer::singleShot(125, gameWindow, [gameWindow]() {
        gameWindow->init();
    });

    /* ⚠️ Parar música antes de cerrar
    if (backgroundPlayer) {
        backgroundPlayer->stop();
        backgroundPlayer->deleteLater();
        backgroundPlayer = nullptr;
    }
    */

    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (w != gameWindow) {
            w->close();
        }
    }
}
