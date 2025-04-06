#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QLabel>      // For QLabel class
#include <QPixmap>     // For QPixmap class
#include <QSize>       // For QSize class
#include <QTransform>  // For QTransform class
#include "icon.h"

class GameWindow : public QWidget // Ensure GameWindow inherits from QWidget
{
    Q_OBJECT
public:
    GameWindow(int type, int fondo);

private:
    int bg; // Number that indicates which skin of the background is being used [0,1,2...]
    int gameType; // Number that indicates whether the game is 1v1, 2v2, friendly, or normal.
    int cardSize;

    // 0 -> 1v1 Ranked
    // 1 -> 1v1 Friendly
    // 2 -> 2v2 (Ranked or Friendly)

    void setBackground(); // Function to set the background based on the bg value
    void setupUI();
    void setupGameElements();
    void resizeEvent(QResizeEvent *event);
    void repositionOrnaments();
    void repositionOptions();

    // Labels to display the corner ornaments
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;

    // Icons
    Icon *settings;
    Icon *chat;
    Icon *quit;

    //Barra
    QFrame *optionsBar;

    QSize ornamentSize; // Size of the corner ornaments
};

#endif // GAMEWINDOW_H
