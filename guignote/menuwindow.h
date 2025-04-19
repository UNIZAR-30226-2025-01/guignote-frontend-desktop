#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QSize>
#include <QLabel>
#include <QFrame>
#include <QMediaPlayer>
#include <QtMultimedia>
#include <QAudioOutput>
#include <QResizeEvent>
#include <QEvent>
#include <QPushButton>
#include <QWebSocket>

class ImageButton;
class Icon;

namespace Ui {
class MenuWindow;
}

class MenuWindow : public QWidget {
    Q_OBJECT
public:
    explicit MenuWindow(QWidget *parent = nullptr);
    QMediaPlayer *backgroundPlayer;
    QAudioOutput *audioOutput;
    ~MenuWindow();

public slots:
    void setVolume(int volumePercentage);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;  // Sobrescribimos closeEvent
    void showEvent(QShowEvent *event) override;


private:

    Ui::MenuWindow *ui;

    // Botones de modos de juego
    ImageButton *boton1v1;
    ImageButton *boton2v2;

    // Barras de la interfaz
    QFrame *bottomBar;
    QFrame *topBar;

    // Iconos de opciones (Settings, Friends, Exit, Inventory)
    Icon *settings;
    Icon *friends;
    Icon *exit;
    Icon *inventory;
    Icon *rankings;
    QPushButton *invisibleButton;
    QLabel *usrLabel;

    // Adornos decorativos en las esquinas
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QSize ornamentSize;

    // Métodos de reposicionamiento de elementos
    void repositionOrnaments();
    void repositionImageButtons();
    void repositionBars();
    void repositionIcons();
    void getSettings();

    // Métodos para la conexión con el backend
    QString loadAuthToken();
    QString token;

    QWebSocket *webSocket;
    void jugarPartida(const QString &token, int capacidad = 2);
    QLabel *mensajeCola = nullptr;
    QDialog *searchingDialog = nullptr;
    int jugadoresCola = 0;
    int jugadoresMax = 0;
    void manejarMensaje(const QString &mensaje);
    QString usr;
    int id;
};

#endif // MENUWINDOW_H
