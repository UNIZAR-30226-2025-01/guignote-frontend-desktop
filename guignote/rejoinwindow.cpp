#include "rejoinwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

RejoinWindow::RejoinWindow(QJsonArray jsonArray, QWidget *parent)
    : QDialog(parent), salas(jsonArray) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(850, 680);
    setupUI();
}

void RejoinWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);
    mainLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(createHeaderLayout());
    populateSalas();

    setLayout(mainLayout);
}

QHBoxLayout* RejoinWindow::createHeaderLayout() {
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

void RejoinWindow::populateSalas() {
    for (const QJsonValue &val : salas) {

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
        connect(btn, &QPushButton::clicked, this, [idStr]() {
            qDebug() << "Sala" << idStr << "pulsada";
        });

        // ——— Montamos la fila dentro del contenedor ———
        row->addWidget(label);
        row->addStretch();
        row->addWidget(btn);
        container->setLayout(row);

        // ——— Añadimos un pequeño margen vertical entre contenedores ———
        mainLayout->addWidget(container);
        mainLayout->setSpacing(10);
    }
}


