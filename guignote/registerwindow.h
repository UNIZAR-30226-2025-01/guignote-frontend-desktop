#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();
};

#endif // REGISTERWINDOW_H
