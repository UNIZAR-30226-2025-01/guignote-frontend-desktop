#include <QApplication>
#include <QtTest>
#include "test_mainwindow.h"
#include "test_registerwindow.h"
#include "test_loginwindow.h"
#include "test_menuwindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;
    // Ejecutar tests de MainWindow
    status |= QTest::qExec(new TestMainWindow, argc, argv);

    // Ejecutar tests de RegisterWindow
    status |= QTest::qExec(new TestRegisterWindow, argc, argv);

    // Ejecutar tests de LoginWindow
    status |= QTest::qExec(new TestLoginWindow,   argc, argv);

    status |= QTest::qExec(new TestMenuWindow,    argc, argv);

    return status;
}
