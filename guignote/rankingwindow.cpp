#include "rankingwindow.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCheckBox>

RankingWindow::RankingWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #171718; border-radius: 30px; padding: 20px;");
    setFixedSize(900, 650);

    networkManager = new QNetworkAccessManager(this);
    setupUI();
}

void RankingWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignTop);

    // Encabezado
    QHBoxLayout *headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("Rankings", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");

    closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/cross.png"));
    closeButton->setIconSize(QSize(22, 22));
    closeButton->setFixedSize(35, 35);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #c2c2c3; border: none; border-radius: 17px; }"
        "QPushButton:hover { background-color: #9b9b9b; }"
        );
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // Controles de filtro
    individualButton = new QPushButton("Individual", this);
    parejasButton = new QPushButton("Parejas", this);
    soloAmigosCheck = new QCheckBox("Solo amigos", this);

    QString buttonStyle =
        "QPushButton {"
        "   background-color: #c2c2c3;"
        "   color: #171718;"
        "   border-radius: 15px;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   padding: 12px 25px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #9b9b9b;"
        "}";

    QString checkboxStyle =
        "QCheckBox {"
        "   color: #ffffff;"
        "   font-size: 14px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 16px;"
        "   height: 16px;"
        "}"
        "QCheckBox::indicator:unchecked {"
        "   background-color: #c2c2c3;"
        "   border: 1px solid #545454;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color: #c2c2c3;"
        "   border: 1px solid #545454;"
        "   image: url(:/icons/cross.png);"
        "}";

    individualButton->setStyleSheet(buttonStyle);
    parejasButton->setStyleSheet(buttonStyle);
    soloAmigosCheck->setStyleSheet(checkboxStyle);

    // Layout solo para los botones
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(individualButton);
    buttonsLayout->addSpacing(15);
    buttonsLayout->addWidget(parejasButton);

    QWidget *buttonsContainer = new QWidget(this);
    buttonsContainer->setLayout(buttonsLayout);

    // Layout general de filtros (botones centrados y checkbox a la derecha)
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addStretch();
    filterLayout->addWidget(buttonsContainer);
    filterLayout->addSpacing(15);
    filterLayout->addWidget(soloAmigosCheck);
    filterLayout->addStretch();

    mainLayout->addLayout(filterLayout);
}
