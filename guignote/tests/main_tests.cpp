#include <QApplication>
#include <QtTest>
#include "test_mainwindow.h"
#include "test_registerwindow.h"
#include "test_loginwindow.h"
#include "test_menuwindow.h"
#include "test_userprofilewindow.h"
#include "test_settingswindow.h"
#include "test_rankingwindow.h"
#include "test_myprofilewindow.h"
#include "test_inventorywindow.h"
#include "test_friendswindow.h"


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

    // Ejecutar tests de MenuWindow
    status |= QTest::qExec(new TestMenuWindow,    argc, argv);

    // Ejecutar tests de UserProfileWindow
    status |= QTest::qExec(new TestUserProfileWindow,   argc, argv);

    // Ejecutar tests de SettingsWindow
    status |= QTest::qExec(new TestSettingsWindow,   argc, argv);

    // Ejecutar tests de RankingWindow
    status |= QTest::qExec(new TestRankingWindow,   argc, argv);

    // Ejecutar tests de MyProfileWindow
    status |= QTest::qExec(new TestMyProfileWindow,   argc, argv);

    // Ejecutar tests de InventoryWindow
    QTest::qExec(new TestInventoryWindow, argc, argv);

    // Ejecutar tests de FriendsWindow
    QTest::qExec(new TestFriendsWindow,   argc, argv);

    return status;
}
