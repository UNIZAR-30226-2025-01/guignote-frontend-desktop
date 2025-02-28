#include "mainwindow.h"
#include "menu.h" // SOLO PARA DEBUG, BORRAR LUEGO
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    menu w;
    w.show();
    return a.exec();
}
