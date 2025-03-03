#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void volverClicked(); // Señal para ocultar la ventana cuando toque
};

#endif // REGISTERWINDOW_H
