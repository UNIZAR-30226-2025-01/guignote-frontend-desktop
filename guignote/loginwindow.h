#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    // Se emite cuando el usuario pulsa “¿No tienes cuenta? Crea una”
    void openRegisterRequested();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *backgroundOverlay;
};

#endif // LOGINWINDOW_H
