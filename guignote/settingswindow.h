#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>

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
    QSettings settings;
    void updateGraphicsMode();
    void loadSettings();  // Cargar configuraciones guardadas
    void saveSettings();  // Guardar configuraciones al salir

};

#endif // SETTINGSWINDOW_H
