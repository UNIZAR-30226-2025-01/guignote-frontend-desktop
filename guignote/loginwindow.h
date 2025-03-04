#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void volverClicked(); // Señal para notificar que se pulsó "Volver"

private:
    // Para gestionar el estado de visibilidad de la contraseña
    bool passwordHidden;
};

#endif // LOGINWINDOW_H
