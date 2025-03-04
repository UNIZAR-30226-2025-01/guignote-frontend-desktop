#include "mainwindow.h"
#include "menuwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MenuWindow w;
    w.showFullScreen();
    return a.exec();
}
