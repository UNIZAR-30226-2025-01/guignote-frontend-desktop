#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>

class RegisterWindow : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    // Se emite cuando el usuario pulsa “¿Ya tienes cuenta? Inicia sesión”
    void openLoginRequested();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *backgroundOverlay;

    // Aquí irán todos los widgets y lógica de registro
};

#endif // REGISTERWINDOW_H
