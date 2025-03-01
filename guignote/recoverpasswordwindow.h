#ifndef RECOVERPASSWORDWINDOW_H
#define RECOVERPASSWORDWINDOW_H

#include <QWidget>

class RecoverPasswordWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RecoverPasswordWindow(QWidget *parent = nullptr);
    ~RecoverPasswordWindow();
};

#endif // RECOVERPASSWORDWINDOW_H
