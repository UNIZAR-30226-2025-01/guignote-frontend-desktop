#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString styleSheet = R"(
        QWidget {
            background-color: #afc7b2;
            color: white;
            font-size: 14px;
            font-family: Arial, sans-serif;
        }

        QLabel {
            font-size: 16px;
            color: #E0C9A6;
        }

        QLineEdit {
            background-color: #f5dada;
            border: 2px solid #c5b58a;
            border-radius: 5px;
            padding: 5px;
            color: white;
        }

        QLineEdit:focus {
            border: 2px solid #abc2c9;
        }

        QPushButton {
            background-color: #86a98a;
            border: none;
            border-radius: 5px;
            padding: 10px;
            font-size: 16px;
            color: white;
        }

        QPushButton:hover {
            background-color: #86a98a;
        }

        QPushButton:pressed {
            background-color: #731008;
        }

        QCheckBox {
            color: #a280b4;
        }
    )";

    this->setStyleSheet(styleSheet);

    QPushButton *togglePasswordButton = new QPushButton(this);
    togglePasswordButton->setIcon(QIcon(":/icons/show_password.png"));
    togglePasswordButton->setCursor(Qt::PointingHandCursor);
    togglePasswordButton->setStyleSheet("border: none; background: transparent;");

    QHBoxLayout *passwordLayout = new QHBoxLayout(ui->passwordLineEdit);
    passwordLayout->addStretch();
    passwordLayout->addWidget(togglePasswordButton);
    passwordLayout->setContentsMargins(0, 0, 5, 0);
    ui->passwordLineEdit->setLayout(passwordLayout);

    // Conectar la acción del botón
    connect(togglePasswordButton, &QPushButton::clicked, this, [=]() mutable {
        bool isPasswordVisible = (ui->passwordLineEdit->echoMode() == QLineEdit::Normal);
        if (isPasswordVisible) {
            ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
            togglePasswordButton->setIcon(QIcon(":/icons/show_password.png"));
        } else {
            ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);
            togglePasswordButton->setIcon(QIcon(":/icons/hide_password.png"));
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
