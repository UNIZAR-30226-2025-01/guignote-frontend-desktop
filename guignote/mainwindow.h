#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openLoginWindow();
    void openRegisterWindow();

    // Estos slots se conectarán a las señales emitidas por RegisterWindow o LoginWindow
    void handleOpenLoginRequested();    // Cuando en RegisterWindow pulsen “¿Ya tienes cuenta?”
    void handleOpenRegisterRequested(); // Cuando en LoginWindow pulsen “¿No tienes cuenta?”

private:
    Ui::MainWindow *ui;

    // Labels para las esquinas, según tu diseño original
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;

    QSize ornamentSize;

    void resizeEvent(QResizeEvent *event) override;
    void repositionOrnaments();
};

#endif // MAINWINDOW_H
