/**
 * @file menuwindow.cpp
 * @brief Implementación de la clase MenuWindow.
 *
 * La clase MenuWindow define la ventana principal del menú de la aplicación.
 * Se configuran el fondo, los botones de selección de modos de juego, los adornos decorativos
 * y se gestionan los redimensionamientos para mantener una disposición coherente de los elementos.
 */

 #include "menuwindow.h"
 #include "icon.h"
 #include "ui_menuwindow.h"
 #include "imagebutton.h"
 #include "inventorywindow.h"
 #include "settingswindow.h"
 #include <QMessageBox>
 #include "friendswindow.h"
 #include "myprofilewindow.h"
 #include "rankingwindow.h"
 #include "rejoinwindow.h"
 #include "customgameswindow.h"
 #include <QGraphicsDropShadowEffect>
 #include <QTimer>
 #include <QNetworkAccessManager>
 #include <QNetworkReply>
 #include <QJsonDocument>
 #include <QJsonObject>
 #include <QJsonArray>
 #include <QFile>
 #include <QStandardPaths>
 #include <QSettings>
 #include <QDebug>
 #include <QPropertyAnimation>
 #include <QGraphicsOpacityEffect>
 #include <QWebSocket>
 #include <QUrl>
 #include <QWebSocketProtocol>
 #include <QApplication>
#include "rankswindow.h"
 
 // Función auxiliar para crear un diálogo modal de sesión expirada.
 static QDialog* createExpiredDialog(QWidget *parent) {
     QDialog *expiredDialog = new QDialog(parent);
     expiredDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
     expiredDialog->setStyleSheet("QDialog { background-color: #171718; border-radius: 5px; padding: 20px; }");
 
     QGraphicsDropShadowEffect *dialogShadow = new QGraphicsDropShadowEffect(expiredDialog);
     dialogShadow->setBlurRadius(10);
     dialogShadow->setColor(QColor(0, 0, 0, 80));
     dialogShadow->setOffset(4, 4);
     expiredDialog->setGraphicsEffect(dialogShadow);
 
     QVBoxLayout *dialogLayout = new QVBoxLayout(expiredDialog);
     QLabel *expiredLabel = new QLabel("Su sesión ha caducado, por favor, vuelva a iniciar sesión.", expiredDialog);
     expiredLabel->setWordWrap(true);
     expiredLabel->setStyleSheet("color: white; font-size: 16px;");
     expiredLabel->setAlignment(Qt::AlignCenter);
     dialogLayout->addWidget(expiredLabel);
 
     QPushButton *okButton = new QPushButton("OK", expiredDialog);
     okButton->setStyleSheet(
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
         "}"
         );
     okButton->setFixedSize(100, 40);
 
     QHBoxLayout *btnLayout = new QHBoxLayout();
     btnLayout->addStretch();
     btnLayout->addWidget(okButton);
     btnLayout->addStretch();
     dialogLayout->addLayout(btnLayout);
 
     QObject::connect(okButton, &QPushButton::clicked, [expiredDialog]() {
         expiredDialog->close();
         qApp->quit();
     });
 
     expiredDialog->adjustSize();
     expiredDialog->move(parent->geometry().center() - expiredDialog->rect().center());
     return expiredDialog;
 }
 
 void MenuWindow::jugarPartida(const QString &userKey, const QString &token, int capacidad) {
     qDebug() << "Uniendose a partida";
     QString url = QString("ws://188.165.76.134:8000/ws/partida/?token=%1&capacidad=%2&es_personalizada=%3")
            .arg(token)
            .arg(capacidad)
            .arg("false");
 
     // Creamos la nueva ventana (EstadoPartida o GameWindow)
     EstadoPartida *gameWindow = new EstadoPartida(usr, userKey, url, /** tapete */ 1, /** skin */1, [userKey]() {
         auto* menu = new MenuWindow(userKey);
         menu->showFullScreen();
     });
 
     gameWindow->setAttribute(Qt::WA_DeleteOnClose);
     gameWindow->setWindowFlag(Qt::Window); // Asegura que sea una ventana independiente
 
     // Le damos la misma geometría que el menú actual
     gameWindow->showFullScreen();
 
     QTimer::singleShot(125, gameWindow, [gameWindow]() {
         gameWindow->init();
     });
 
     // ⚠️ Parar música antes de cerrar
     if (backgroundPlayer) {
         backgroundPlayer->stop();
         backgroundPlayer->deleteLater();
         backgroundPlayer = nullptr;
     }

     for (QWidget *w : QApplication::topLevelWidgets()) {
         if (w != gameWindow) {
             w->close();
         }
     }
 }
 
 void MenuWindow::checkRejoin() {
     QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);

     // Primer consulta: salas reconectables
     QNetworkRequest requestReconectables(QUrl("http://188.165.76.134:8000/salas/reconectables/"));
     requestReconectables.setRawHeader("Auth", token.toUtf8());
     qDebug() << "Comprobamos salas reconectables...";

     QNetworkReply *replyReconectables = networkManager->get(requestReconectables);

     // Segunda consulta: salas pausadas
     QNetworkRequest requestPausadas(QUrl("http://188.165.76.134:8000/salas/pausadas/"));
     requestPausadas.setRawHeader("Auth", token.toUtf8());
     qDebug() << "Comprobamos salas pausadas...";

     QNetworkReply *replyPausadas = networkManager->get(requestPausadas);

     connect(replyReconectables, &QNetworkReply::finished, this, [this, replyReconectables, replyPausadas]() {
         // Comprobación de errores para salas reconectables
         if (replyReconectables->error() != QNetworkReply::NoError) {
             qDebug() << "Error de red (reconectables):" << replyReconectables->errorString();
             replyReconectables->deleteLater();
             return;
         }

         const QByteArray dataReconectables = replyReconectables->readAll();
         QJsonParseError parseErrorReconectables;
         QJsonDocument docReconectables = QJsonDocument::fromJson(dataReconectables, &parseErrorReconectables);
         if (parseErrorReconectables.error != QJsonParseError::NoError || !docReconectables.isObject()) {
             qDebug() << "Error parseando JSON (reconectables):" << parseErrorReconectables.errorString();
             replyReconectables->deleteLater();
             return;
         }

         QJsonObject objReconectables = docReconectables.object();
         if (!objReconectables.contains("salas") || !objReconectables.value("salas").isArray()) {
             qDebug() << "La respuesta de salas reconectables no contiene el campo 'salas'";
             replyReconectables->deleteLater();
         } else {
             salas = objReconectables.value("salas").toArray();
         }

         // Comprobación de errores para salas pausadas
         if (replyPausadas->error() != QNetworkReply::NoError) {
             qDebug() << "Error de red (pausadas):" << replyPausadas->errorString();
             replyPausadas->deleteLater();
             return;
         }

         const QByteArray dataPausadas = replyPausadas->readAll();
         QJsonParseError parseErrorPausadas;
         QJsonDocument docPausadas = QJsonDocument::fromJson(dataPausadas, &parseErrorPausadas);
         if (parseErrorPausadas.error != QJsonParseError::NoError || !docPausadas.isObject()) {
             qDebug() << "Error parseando JSON (pausadas):" << parseErrorPausadas.errorString();
             replyPausadas->deleteLater();
             return;
         }

         QJsonObject objPausadas = docPausadas.object();
         if (!objPausadas.contains("salas") || !objPausadas.value("salas").isArray()) {
             qDebug() << "La respuesta de salas pausadas no contiene el campo 'salas'";
             replyPausadas->deleteLater();
         } else {
             salasPausadas = objPausadas.value("salas").toArray();
         }

         // Lógica para mostrar el botón
         if (salas.isEmpty() && salasPausadas.isEmpty()) {
             qDebug() << "No hay salas reconectables ni pausadas.";
             ReconnectButton->hide();
         } else {
             qDebug() << "Salas reconectables o pausadas encontradas:";
             ReconnectButton->show();
         }

         replyReconectables->deleteLater();
         replyPausadas->deleteLater();
     });
 }

 
 // Constructor de la clase MenuWindow
 MenuWindow::MenuWindow(const QString &userKey, QWidget *parent) :
     QWidget(parent),
     ui(new Ui::MenuWindow),
     boton1v1(nullptr),
     boton2v2(nullptr),
     bottomBar(nullptr),
     topBar(nullptr),
     settings(nullptr),
     friends(nullptr),
     exit(nullptr),
     usrLabel(nullptr),
     rejoinTimer(new QTimer(this))
 {
     this->setAttribute(Qt::WA_DeleteOnClose);
     connect(rejoinTimer, &QTimer::timeout,
             this, &MenuWindow::checkRejoin);
     rejoinTimer->start(1000);
 
     ui->setupUi(this);
     // Activa el relleno de fondo desde la hoja de estilo
     this->setAttribute(Qt::WA_StyledBackground, true);
 
     this->userKey = userKey;
     token = loadAuthToken(userKey);
     qDebug() << "Token recibido: " + token;
 
     // ------------- IMÁGENES DE CARTAS -------------
     boton1v1 = new ImageButton(":/images/cartaBoton.png", "Individual", this);
     boton2v2 = new ImageButton(":/images/cartasBoton.png", "Parejas", this);
 
     // ------------- EVENTOS DE CLICK EN CARTAS -------------
     connect(boton1v1, &ImageButton::clicked, this, [this, userKey]() {
         if (!nameHasLoaded) return;
         jugarPartida(userKey, token, 2);
     });
     connect(boton2v2, &ImageButton::clicked, this, [this, userKey]() {
         if (!nameHasLoaded) return;
         jugarPartida(userKey, token, 4);
     });
 
      // ------------- BOTON DE RECONEXION -------------
 
     // Layout principal
     mainLayout = new QVBoxLayout;
     // agregamos un espacio creciente para empujar el botón abajo
     mainLayout->addStretch();
 
     // Creamos ya el botón (pero oculto)
     ReconnectButton = new QPushButton("Reconectarse", this);
     ReconnectButton->setVisible(false);

     ReconnectButton->setStyleSheet(R"(
    QPushButton {
        background-color: #171718;  /* Fondo oscuro */
        color: white;               /* Color del texto */
        border: 2px solid #333333;  /* Borde oscuro */
        border-radius: 5px;         /* Esquinas redondeadas */
    }

    QPushButton:hover {
        background-color: #2a2a2b;  /* Fondo más claro al pasar el ratón */
    }
    )");
 
     // Establecer una altura y anchura fija para el botón
     ReconnectButton->setFixedSize(200, 50);  // Establece un tamaño fijo
 
     // Conectar la señal del botón con el slot
     connect(ReconnectButton, &QPushButton::clicked, this, [this]() {
         if (!nameHasLoaded) return;
         RejoinWindow *rjWin = new RejoinWindow(
             this->salas,
             this->salasPausadas,
             this->fondo,
             this->userKey,
             this->usr,
             this
             );
         rjWin->setModal(true);
         rjWin->exec();
     });
 
     QHBoxLayout *h = new QHBoxLayout;
     h->addStretch();
     h->addWidget(ReconnectButton);
     h->addStretch();
     mainLayout->addLayout(h);
     mainLayout->setContentsMargins(0,0,0,90);
     mainLayout->setAlignment(Qt::AlignBottom);
 
     setLayout(mainLayout);
 
     checkRejoin();
 
     // ------------- BARRAS (BARS) -------------
     bottomBar = new QFrame(this);
     topBar = new QFrame(this);
     topBar->setObjectName("topBar");
     bottomBar->setObjectName("bottomBar");
 
     // ------------- DETECTAR CLICKS EN TOPBAR -------------
     invisibleButton = new QPushButton(this);
     invisibleButton->setStyleSheet("background: transparent; border: none;");
     invisibleButton->setCursor(Qt::PointingHandCursor);
     connect(invisibleButton, &QPushButton::clicked, [=]() {
         if (!nameHasLoaded) return;
         MyProfileWindow *profileWin = new MyProfileWindow(userKey, this);
         profileWin->setModal(true);
         profileWin->show();
     });
 
     // ------------- NOMBRE DE USUARIO Y RANGO EN TOPBAR -------------
     usrLabel = new QLabel(this);
     QTimer::singleShot(1000, this, [this]() {
         // Se carga el token desde el archivo de configuración
         if (token.isEmpty()) {
             usrLabel->setText("ERROR");
         } else {
             QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
             QNetworkRequest request(QUrl("http://188.165.76.134:8000/usuarios/estadisticas/"));
             request.setRawHeader("Auth", token.toUtf8());
 
             QNetworkReply *reply = networkManager->get(request);
             connect(reply, &QNetworkReply::finished, this, [this, reply]() {
                 int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                 if (statusCode == 401) {
                     createExpiredDialog(this)->show();
                     reply->deleteLater();
                     return;
                 }
                 if (reply->error() == QNetworkReply::NoError) {
                     QByteArray responseData = reply->readAll();
                     QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
                     QJsonObject jsonObj = jsonDoc.object();
 
                     // Se extrae el nombre del usuario y otros datos
                     QString nombre = jsonObj.value("nombre").toString();
                     this->usr = nombre;
                     int ELO = 0;            // Actualiza si dispones de este dato
                     QString rank = "Rango"; // Actualiza si se recibe el rango
 
                     QString UsrELORank = QString(
                                              "<span style='font-size: 24px; font-weight: bold; color: white;'>%1 (%2) </span>"
                                              "<span style='font-size: 20px; font-weight: normal; color: white;'>%3</span>"
                                              ).arg(nombre).arg(ELO).arg(rank);
 
                     usrLabel->setText(UsrELORank);

                     nameHasLoaded = true;
 
                     // ------------- MÚSICA -------------
                     backgroundPlayer = new QMediaPlayer(this);
                     audioOutput = new QAudioOutput(this);
                     backgroundPlayer->setAudioOutput(audioOutput);
                     audioOutput->setVolume(0.0); // Rango 0.0 a 1.0
                     backgroundPlayer->setSource(QUrl("qrc:/bgm/menu_jazz_lofi.mp3"));
                     backgroundPlayer->setLoops(QMediaPlayer::Infinite);
                     backgroundPlayer->play();
                     getSettings();
                 } else {
                     usrLabel->setText("Error al cargar usuario");
                 }
                 reply->deleteLater();
             });
         }
     });
     usrLabel->setAlignment(Qt::AlignCenter);
     usrLabel->setTextFormat(Qt::RichText);
     usrLabel->setStyleSheet("color: white; background: transparent;");
 
     // ------------- ICONOS -------------
     settings = new Icon(this);
     friends = new Icon(this);
     exit = new Icon(this);
     inventory = new Icon(this);
     rankings = new Icon(this);
     customGames = new Icon(this);
     ranks    = new Icon(this);
 
     settings->setImage(":/icons/audio.png", 50, 50);
     friends->setImage(":/icons/friends.png", 60, 60);
     exit->setImage(":/icons/door.png", 60, 60);
     inventory->setImage(":/icons/chest.png", 50, 50);
     rankings->setImage(":/icons/trophy.png", 50, 50);
     customGames->setImage(":/icons/gameslist.png", 50, 50);
     ranks    ->setImage(":/icons/ranks.png", 50, 50);
 
     // ------------- EVENTOS DE CLICK EN ICONOS -------------
     connect(settings, &Icon::clicked, [=]() {
         if (!nameHasLoaded) return;
         settings->setImage(":/icons/darkenedaudio.png", 60, 60);
         SettingsWindow *settingsWin = new SettingsWindow(this, this, usr);
         settingsWin->setModal(true);
         connect(settingsWin, &QDialog::finished, [this](int){
             settings->setImage(":/icons/audio.png", 60, 60);
         });
         settingsWin->exec();
     });
     connect(friends, &Icon::clicked, this, [=]() {
         if (!nameHasLoaded) return;
         friends->setImage(":/icons/darkenedfriends.png", 60, 60);
         auto *fw = new friendswindow(userKey, this);
         fw->setModal(true);
         fw->exec();
         friends->setImage(":/icons/friends.png", 60, 60);
     });
 
     connect(exit, &Icon::clicked, this, [this]() {
         if (!nameHasLoaded) return;
         exit->setImage(":/icons/darkeneddoor.png", 60, 60);
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
         connect(yesButton, &QPushButton::clicked, []() {
             qApp->quit();
         });
         connect(noButton, &QPushButton::clicked, [=]() {
             confirmDialog->close();
         });
         connect(confirmDialog, &QDialog::finished, [=](int) {
             exit->setImage(":/icons/door.png", 60, 60);
         });
         confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
         confirmDialog->show();
 
         // QTimer para mantener el diálogo centrado
         QTimer *centerTimer = new QTimer(confirmDialog);
         centerTimer->setInterval(50);
         connect(centerTimer, &QTimer::timeout, [this, confirmDialog]() {
             confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
         });
         centerTimer->start();
     });
     connect(ranks, &Icon::clicked, this, [=]() {
         // (Versión sin imagen “oscurecida”; ajusta si añades una.)
         RanksWindow *rw = new RanksWindow(userKey, this);
         rw->setModal(true);
         rw->exec();
     });

     connect(inventory, &Icon::clicked, this, [this]() {
         if (!nameHasLoaded) return;
         inventory->setImage(":/icons/darkenedchest.png", 60, 60);
         InventoryWindow *inventoryWin = new InventoryWindow(this,usr);
         inventoryWin->setModal(true);
         connect(inventoryWin, &QDialog::finished, [this](int){
             inventory->setImage(":/icons/chest.png", 60, 60);
         });
         inventoryWin->exec();
     });
     connect(rankings, &Icon::clicked, this, [this, userKey]() {
         if (!nameHasLoaded) return;
         rankings->setImage(":/icons/darkenedtrophy.png", 60, 60);
 
         RankingWindow *rankingWin = new RankingWindow(userKey, this);
         rankingWin->setModal(true);
 
         connect(rankingWin, &QDialog::finished, [this, rankingWin](int){
             rankings->setImage(":/icons/trophy.png", 60, 60);
         });
 
         rankingWin->exec();
     });
     connect(customGames, &Icon::clicked, this, [userKey, this](){
         if (!nameHasLoaded) return;
         customGames->setImage(":/icons/darkenedgameslist.png", 50, 50);

         // Crear pantalla nueva aquí.
         CustomGamesWindow *cstgmWin = new CustomGamesWindow(userKey, usr, fondo, this);
         cstgmWin->setModal(true);

         connect(cstgmWin, &QDialog::finished, [this, cstgmWin](int){
             customGames->setImage(":/icons/gameslist.png", 50, 50);
         });

         cstgmWin->exec();
     });
 
     // ------------- ORNAMENTOS ESQUINAS -------------
     ornamentSize = QSize(300, 299);
     QPixmap ornamentPixmap(":/images/set-golden-border-ornaments/gold_ornaments.png");
 
     cornerTopLeft = new QLabel(this);
     cornerTopRight = new QLabel(this);
     cornerBottomLeft = new QLabel(this);
     cornerBottomRight = new QLabel(this);
 
     cornerTopLeft->setPixmap(ornamentPixmap.scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
 
     QTransform transformH;
     transformH.scale(-1, 1);
     cornerTopRight->setPixmap(
         ornamentPixmap.transformed(transformH, Qt::SmoothTransformation)
             .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
         );
 
     QTransform transformV;
     transformV.scale(1, -1);
     cornerBottomLeft->setPixmap(
         ornamentPixmap.transformed(transformV, Qt::SmoothTransformation)
             .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
         );
 
     QTransform transformHV;
     transformHV.scale(-1, -1);
     cornerBottomRight->setPixmap(
         ornamentPixmap.transformed(transformHV, Qt::SmoothTransformation)
             .scaled(ornamentSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
         );
 
     QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
     for (QLabel* corner : corners) {
         corner->setFixedSize(ornamentSize);
         corner->setAttribute(Qt::WA_TransparentForMouseEvents);
         corner->setAttribute(Qt::WA_TranslucentBackground);
         corner->setStyleSheet("background: transparent;");
         corner->raise();
     }
 
     this->setStyleSheet(R"(
     /* Fondo de la ventana con gradiente verde */
     QWidget {
         background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                     fx:0.5, fy:0.5,
                                     stop:0 #1f5a1f,
                                     stop:1 #0a2a08);
     }
 
     /* Barras top y bottom con gradiente vertical gris–negro */
     QFrame#topBar, QFrame#bottomBar {
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
 
     repositionOrnaments();
 }
 
 // Función para extraer el token de autenticación desde el archivo .conf
 QString MenuWindow::loadAuthToken(const QString &userKey) {
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
  * @brief Reposiciona los adornos decorativos en las esquinas de la ventana.
  */
 void MenuWindow::repositionOrnaments() {
     int w = this->width();
     int h = this->height();
     int topOffset = 0;
 
     cornerTopLeft->move(0, topOffset);
     cornerTopRight->move(w - cornerTopRight->width(), topOffset);
     cornerBottomLeft->move(0, h - cornerBottomLeft->height());
     cornerBottomRight->move(w - cornerBottomRight->width(), h - cornerBottomRight->height());
 
     QList<QLabel*> corners = {cornerTopLeft, cornerTopRight, cornerBottomLeft, cornerBottomRight};
     for (QLabel* corner : corners) {
         corner->lower();
     }
 }
 
 // Función para reposicionar los ImageButtons
 void MenuWindow::repositionImageButtons() {
     int w = this->width();
     int h = this->height();
     int buttonSpacing = w / 10;
 
     QSize size = boton1v1->updatesize(h);
     size = boton2v2->updatesize(h);
     int buttonWidth = size.width();
     int buttonHeight = size.height();
 
     int totalWidth = (2 * buttonWidth) + buttonSpacing;
     int startX = (w - totalWidth) / 2;
     int startY = (h - buttonHeight) / 2;
 
     boton1v1->move(startX, startY);
     boton2v2->move(startX + buttonWidth + buttonSpacing, startY);
 }
 
 // Función para reposicionar las barras (topBar y bottomBar)
 void MenuWindow::repositionBars() {
     int w = this->width();
     int barWidthTop = w / 3;
     int barWidthBottom = w * 0.3;
     int barHeight = 80;
     int xPosT = (w - barWidthTop) / 2;
     int xPosB = (w - barWidthBottom) / 2;
     int yPos = this->height() - barHeight;
 
     topBar->setGeometry(xPosT, 0, barWidthTop, barHeight);
     bottomBar->setGeometry(xPosB, yPos, barWidthBottom, barHeight);
 
     QPoint topBarPos = topBar->pos();
     invisibleButton->setGeometry(topBarPos.x(), topBarPos.y(), topBar->width(), topBar->height());
 
     int usrLabelHeight = 30;
     int usrLabelWidth = barWidthTop - 20;
     int xUsr = xPosT + 10; // 10px de margen lateral
     int yUsr = topBarPos.y() + (barHeight - usrLabelHeight) / 2;
     usrLabel->setGeometry(xUsr, yUsr, usrLabelWidth, usrLabelHeight);
 
     topBar->lower();
     usrLabel->raise();
     invisibleButton->raise();
 }
 
 // Función para reposicionar los iconos
 void MenuWindow::repositionIcons() {
     // 1) Recolectamos los widgets en un array
     QList<QWidget*> icons = {
         friends,
         customGames,
         rankings,
         inventory,
         ranks,
         settings,
         exit
     };

     // 2) Datos de la barra
     int barX      = bottomBar->pos().x();
     int barY      = bottomBar->pos().y();
     int barWidth  = bottomBar->width();
     int barHeight = bottomBar->height();

     // 3) Sumar anchos de todos los iconos
     int totalIconsWidth = 0;
     for (QWidget* w : icons) {
         totalIconsWidth += w->width();
     }

     // 4) Calcular huecos: (número de widgets + 1)
     int gaps = icons.size() + 1;
     int spacing = (barWidth - totalIconsWidth) / gaps;

     // 5) Calcular Y centrados (exit puede tener altura distinta)
     int yCommon = barY + (barHeight - settings->height()) / 2;
     int yExit   = barY + (barHeight - exit->height())     / 2;

     // 6) Colocar iterativamente
     int x = barX + spacing;
     for (QWidget* w : icons) {
         int y = (w == exit ? yExit : yCommon);
         w->move(x, y);
         x += w->width() + spacing;
     }
 }
 
 // Función para recolocar y reposicionar todos los elementos
 void MenuWindow::resizeEvent(QResizeEvent *event) {
     if (this->width() <= 0 || this->height() <= 0) {
         qWarning() << "Evitar redimensionamiento con tamaño inválido:" << this->width() << "x" << this->height();
         return;
     }
     repositionOrnaments();
     repositionBars();
     repositionImageButtons();
     repositionIcons();
     QWidget::resizeEvent(event);
 }
 
 void MenuWindow::setVolume(int volumePercentage) {
     if (audioOutput) {
         audioOutput->setVolume(volumePercentage / 100.0);
     }
 }
 
 MenuWindow::~MenuWindow() {
     qDebug() << "[DEBUG] MenuWindow destruido.";
     delete ui;
 }
 
 // Sobrescribimos closeEvent para parar la música:
 void MenuWindow::closeEvent(QCloseEvent *event)
 {
     // Detenemos la música antes de cerrar
     if (backgroundPlayer) {
         backgroundPlayer->stop();
     }
 
     // Luego dejamos que continúe el proceso normal de cierre
     event->accept();
 }
 
 void MenuWindow::getSettings() {
     QString config = "Sota, Caballo y Rey_" + usr;
     QSettings settings("Grace Hopper", config);
     int volume = settings.value("sound/volume", 50).toInt();
     qDebug() << "Cargando configuración ["<< usr <<"] - Volumen:" << volume;
 
     this->showFullScreen();
 
     if (audioOutput) {
         audioOutput->setVolume(static_cast<double>(volume) / 100.0);
     } else {
         qWarning() << "Error: audioOutput no está inicializado.";
     }
     this->update();
 }
 
 /**
  * @brief Evento que se ejecuta al mostrar la ventana.
  *
  * Aplica una animación de fade in para que la aparición del menú sea suave.
  */
 void MenuWindow::showEvent(QShowEvent *event) {
     QWidget::showEvent(event);
     QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(this);
     this->setGraphicsEffect(fadeEffect);
     fadeEffect->setOpacity(0.0);
     QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(fadeEffect, "opacity", this);
     fadeInAnimation->setDuration(1500); // Duración mayor para suavizar la transición
     fadeInAnimation->setStartValue(0.0);
     fadeInAnimation->setEndValue(1.0);
     fadeInAnimation->start(QAbstractAnimation::DeleteWhenStopped);
 }
 
