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
    explicit MenuWindow(const QString &userKey, QWidget *parent = nullptr);
    QMediaPlayer *backgroundPlayer = nullptr;
    QAudioOutput *audioOutput = nullptr;
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
    QString loadAuthToken(const QString &userKey);
    QString token;

    QWebSocket *webSocket;
    void jugarPartida(const QString &userKey, const QString &token, int capacidad = 2);
    QLabel *mensajeCola = nullptr;
    QDialog *searchingDialog = nullptr;
    int jugadoresCola = 0;
    int jugadoresMax = 0;
    void manejarMensaje(const QString &userKey, const QString &mensaje);
    QString usr;
    int id;

    QLabel* countLabel = nullptr;

    void ensureLoggedIn();
    QString loadToken();
    void saveToken(const QString &tok);
    Icon *friendsIcon = nullptr;
    Icon *messagesIcon = nullptr;
    int   unreadMessages = 0;
};

#endif // MENUWINDOW_H
