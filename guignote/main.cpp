#include "mainwindow.h"
#include "menuwindow.h"
#include <QApplication>
#include <QSettings>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [](){
        QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
        if (!settings.value("auth/remember", true).toBool()) {
            settings.remove("auth/token");
        }
    });


    // Verificamos si hay token almacenado en QSettings
    QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
    QString token = settings.value("auth/token", "").toString();

    if (!token.isEmpty()) {
        MenuWindow w;
        w.show();
        return a.exec();
    } else {
        MainWindow w;
        w.show();
        return a.exec();
    }
}
