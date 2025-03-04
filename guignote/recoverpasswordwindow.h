#ifndef RECOVERPASSWORDWINDOW_H
#define RECOVERPASSWORDWINDOW_H

#include <QDialog>  // Se hereda de QDialog

class RecoverPasswordWindow : public QDialog
{
    Q_OBJECT
public:
    explicit RecoverPasswordWindow(QWidget *parent = nullptr);
    ~RecoverPasswordWindow();
};

#endif // RECOVERPASSWORDWINDOW_H
