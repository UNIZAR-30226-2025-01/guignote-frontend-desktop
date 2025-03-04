#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

class QLabel;
class QWidget;

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void openRegisterRequested(); // Señal para abrir ventana de registro

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *backgroundOverlay; // Overlay para enfocar la atención en el diálogo
};

#endif // LOGINWINDOW_H
