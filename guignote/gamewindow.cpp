#include "gamewindow.h"
#include "settingswindow.h"
#include "menuwindow.h"
#include "carta.h"
#include "mano.h"
#include "deck.h"
#include "posicion.h"
#include <QLabel>
#include <QPixmap>
#include <QSize>
#include <QTransform>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QTimer>

QMap<QString, Carta*> GameWindow::cartasPorId;

void GameWindow::addCartaPorId(Carta* c){
    cartasPorId[c->idGlobal] = c;
}

Carta* GameWindow::getCartaPorId(QString id){
    return cartasPorId.value(id, nullptr);
}

GameWindow::GameWindow(int type, int fondo) {
    bg = fondo;
    gameType = type;
    cardSize = 150;
    setBackground();
    setupUI();
    setupGameElements();
}

void GameWindow::setupUI() {
    optionsBar = new QFrame(this);
    optionsBar->setObjectName("optionsBar");

    settings = new Icon(this);
    settings->setImage(":/icons/settings.png", 50, 50);
    chat = new Icon(this);
    chat->setImage(":/icons/message.png", 50, 50);
    quit = new Icon(this);
    quit->setImage(":/icons/door.png", 60, 60);

    connect(settings, &Icon::clicked, [=]() {
        settings->setImage(":/icons/darkenedsettings.png", 50, 50);
        SettingsWindow *settingsWin = new SettingsWindow(this, this);
        settingsWin->setModal(true);
        connect(settingsWin, &QDialog::finished, [this](int){
            settings->setImage(":/icons/settings.png", 50, 50);
        });
        settingsWin->exec();
    });

    connect(chat, &Icon::clicked, this, []() {
        qDebug() << "Chat icon clicked!";
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
        connect(yesButton, &QPushButton::clicked, [=]() {
            // Get the current size of MenuWindow
            QSize windowSize = this->size();  // Get the size of MenuWindow

            // Create and show the GameWindow with the same size as MenuWindow
            MenuWindow *menuWindow = new MenuWindow();
            menuWindow->resize(windowSize);  // Set the size of GameWindow to match MenuWindow
            menuWindow->show();

            // Close the current window (MenuWindow)
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

        // QTimer para mantener el diálogo centrado
        QTimer *centerTimer = new QTimer(confirmDialog);
        centerTimer->setInterval(50);
        connect(centerTimer, &QTimer::timeout, [this, confirmDialog]() {
            confirmDialog->move(this->geometry().center() - confirmDialog->rect().center());
        });
        centerTimer->start();
    });
}

void GameWindow::setBackground() {
    QString ornament;
    switch (bg) {
    case 0:
        ornament = ":/images/set-golden-border-ornaments/gold_ornaments.png";
        // Set background to the original green gradient pattern
        this->setStyleSheet(R"(
        /* Fondo de la ventana con gradiente verde */
        QWidget {
            background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                        fx:0.5, fy:0.5,
                                        stop:0 #1f5a1f,
                                        stop:1 #0a2a08);
        }

    /* Barra con gradiente vertical gris–negro */
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
        break;
    case 1:
        ornament = ":/images/set-golden-border-ornaments/black_ornaments.png";
        // Set background to a red version of the pattern
        this->setStyleSheet(R"(
        /* Fondo de la ventana con gradiente rojo */
        QWidget {
            background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                        fx:0.5, fy:0.5,
                                        stop:0 #5a1f1f,  /* Red start color */
                                        stop:1 #2a0808); /* Dark red end color */
        }

    /* Barra con gradiente vertical gris–negro */
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
        break;
    default:
        ornament = ":/images/set-golden-border-ornaments/gold_ornaments.png";
        // Set background to the original green gradient pattern
        this->setStyleSheet(R"(
        /* Fondo de la ventana con gradiente verde */
        QWidget {
            background: qradialgradient(cx:0.5, cy:0.5, radius:1,
                                        fx:0.5, fy:0.5,
                                        stop:0 #1f5a1f,
                                        stop:1 #0a2a08);
        }

    /* Barra con gradiente vertical gris–negro */
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
        break;
    }

        QPixmap ornamentPixmap(ornament);
        // Decoraciones Esquinas
        ornamentSize = QSize(300, 299);

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
}

void GameWindow::setupGameElements() {
    gameType = 2;
    Carta *testCard1 = new Carta(this, this, "1", "Bastos", cardSize, 0);
    Carta *testCard2 = new Carta(this, this, "10", "Oros", cardSize, 1);
    Carta *testCard3 = new Carta(this, this, "0", "Copas", cardSize, 1);
    manos.append(new Mano(0, 0));
    manos.append(new Mano(1, 1));
    manos[0]->añadirCarta(testCard1);
    manos[0]->añadirCarta(testCard2);
    manos[1]->añadirCarta(testCard3);

    posiciones.append(new Posicion(this, this, cardSize, 0));
    posiciones.append(new Posicion(this, this, cardSize, 1));

    if (gameType == 2) {
        Carta *testCard4 = new Carta(this, this, "0", "Oros", cardSize, 0);
        Carta *testCard5 = new Carta(this, this, "0", "Espadas", cardSize, 1);
        manos.append(new Mano(2, 2));
        manos.append(new Mano(3, 3));
        manos[2]->añadirCarta(testCard4);
        manos[3]->añadirCarta(testCard5);
        posiciones.append(new Posicion(this, this, cardSize, 2));
        posiciones.append(new Posicion(this, this, cardSize, 3));
    }

    Carta *triunfo = new Carta(this, this, "3", "Oros", cardSize, 0);
    deck = new Deck(triunfo, 0, cardSize, this);
}

void GameWindow::repositionHands(){
    for (Mano* mano : manos) mano->mostrarMano();
}

void GameWindow::repositionOrnaments() {
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

void GameWindow::repositionOptions() {
    // Set optionsBar position to the top-right corner of the screen
    int windowWidth = this->width();
    int windowHeight = this->height();

    int x = (windowWidth - windowWidth/8) / 2;  // Center horizontally

    // Place optionsBar at the top-right of the window with a height of 80
    optionsBar->setGeometry(x, 0, windowWidth/8, 80);
    optionsBar->raise();
    settings->raise();
    chat->raise();
    quit->raise();

    // Create a horizontal layout to arrange the icons inside optionsBar
    QHBoxLayout *layout = new QHBoxLayout(optionsBar);

    // Set a minimum space of 5 pixels between icons
    layout->setSpacing(5);

    // Add the three icons to the layout
    layout->addWidget(settings);
    layout->addWidget(chat);
    layout->addWidget(quit);

    // Make the icons stretch to fill the available space
    layout->setStretch(0, 1);
    layout->setStretch(1, 1);
    layout->setStretch(2, 1);

    // Set the layout to the optionsBar
    optionsBar->setLayout(layout);

    // Optionally, adjust the icons' alignment (optional, depending on preference)
    layout->setAlignment(Qt::AlignCenter);  // Align icons in the center horizontally
}



// Función para recolocar y reposicionar todos los elementos
void GameWindow::resizeEvent(QResizeEvent *event) {
    // Simply call QWidget's resizeEvent method
    QWidget::resizeEvent(event);
    repositionOrnaments();
    repositionHands();
    repositionOptions();
    deck->actualizarVisual();

    for (int i = 0; i < posiciones.size(); ++i) {
        posiciones[i]->mostrarPosicion();
    }
}



