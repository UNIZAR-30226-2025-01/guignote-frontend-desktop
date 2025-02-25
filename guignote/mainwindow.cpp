#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // QAction para poder mostrar o esconder la contraseña
    QAction *togglePasswordAction = new QAction(this);
    QIcon showIcon(":/icons/show_password.png");
    QIcon hideIcon(":/icons/hide_password.png");

    // Agregar la acción a la caja donde estará la contraseña
    ui->passwordLineEdit->addAction(togglePasswordAction, QLineEdit::TrailingPosition);

    // Conectar la QAction para permitir alternar la visibilidad de la contraseña
    connect(togglePasswordAction, &QAction::triggered, this, [=]() mutable {
        bool isPasswordVisible = (ui->passwordLineEdit->echoMode() == QLineEdit::Normal);
        if(isPasswordVisible) {
            ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
            togglePasswordAction->setIcon(hideIcon);
        } else {
            ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);
            togglePasswordAction->setIcon(showIcon);
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
