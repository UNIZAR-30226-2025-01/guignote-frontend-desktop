#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

class LoginWindow : public QDialog
{
    Q_OBJECT  // Asegúrate de incluir este macro

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void volverClicked(); // Señal para notificar que se pulsó "Volver"

private:
    bool passwordHidden;
};

#endif // LOGINWINDOW_H
