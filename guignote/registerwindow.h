#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>  // Se hereda de QDialog para popup modal

class RegisterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();
};

#endif // REGISTERWINDOW_H
