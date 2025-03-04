#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

class QListWidget;
class QStackedWidget;
class QRadioButton;
class QPushButton;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

private:
    QListWidget *sidebar;
    QStackedWidget *stackedWidget;
    QRadioButton *radioWindowed;
    QRadioButton *radioFullscreen;
    QPushButton *closeButton;
    void updateGraphicsMode();

};

#endif // SETTINGSWINDOW_H
