#include "ventanasalirpartida.h"

VentanaInfo::VentanaInfo(const QString& msg, std::function<void()> onSalir, QWidget* parent)
    : QWidget(parent), onSalirCallback(onSalir) {

    this->setAttribute(Qt::WA_StyledBackground, true); // necesario para que el styleSheet tenga efecto en QWidget sin layout
    this->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    this->setGeometry(parent->rect());
    this->raise();
    this->show();

    QWidget* popup = new QWidget(this, Qt::FramelessWindowHint);
    popup->setStyleSheet(R"(
        background: qlineargradient(
            x1:0, y1:0,
            x2:0, y2:1,
            stop:0 #1e1e1e,
            stop:1 #000000
        );
        border: 4px solid #666;
        border-radius: 16px;
    )");

    QLabel* label = new QLabel(msg, popup);
    label->setStyleSheet("font-size: 48px; font-weight: bold; color: white; background-color: transparent; border: none;");
    label->setAlignment(Qt::AlignCenter);

    QPushButton* botonSalir = new QPushButton("Salir", popup);
    botonSalir->setFixedSize(132, 56);
    botonSalir->setStyleSheet(R"(
        font-size: 32px;
        background-color: #1f1f1f;
        color: white;
        border-radius: 12px;
        border: 2px solid #666;
        padding: 4px;
    )");

    connect(botonSalir, &QPushButton::clicked, this, [this]() {
        if (onSalirCallback) onSalirCallback();
        this->close();
    });

    QVBoxLayout* layout = new QVBoxLayout(popup);
    layout->addStretch();
    layout->addWidget(label, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    layout->addWidget(botonSalir, 0, Qt::AlignCenter);
    layout->addStretch();

    popup->setLayout(layout);

    QSize screenSize = parent->size();
    int w = screenSize.width() * 0.6;
    int h = screenSize.height() * 0.5;
    popup->setFixedSize(w, h);
    popup->move((screenSize.width() - w) / 2, (screenSize.height() - h) / 2);
    popup->show();
}
