#include "test_friendswindow.h"
#include "friendswindow.h"

#include <QtTest/QtTest>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

void TestFriendsWindow::initTestCase()
{
    // Nada que preparar globalmente
}

void TestFriendsWindow::cleanupTestCase()
{
    // Nada que limpiar
}

void TestFriendsWindow::test_window_flags_and_size()
{
    w = new friendswindow(QStringLiteral("testuser"), nullptr);
    QVERIFY(w->windowFlags() & Qt::FramelessWindowHint);
    QCOMPARE(w->size(), QSize(900, 650));
    delete w; w = nullptr;
}

void TestFriendsWindow::test_basic_ui_elements()
{
    w = new friendswindow(QStringLiteral("testuser"), nullptr);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));

    // Título
    QLabel *title = w->findChild<QLabel*>();
    QVERIFY(title);
    QVERIFY(title->text().contains("Menú de Amigos"));

    // Botón cerrar
    QPushButton *btnClose = w->findChild<QPushButton*>();
    QVERIFY(btnClose);
    QCOMPARE(btnClose->iconSize(), QSize(22,22));
    QCOMPARE(btnClose->size(), QSize(35,35));

    delete w; w = nullptr;
}

void TestFriendsWindow::test_tab_count_and_titles()
{
    w = new friendswindow(QStringLiteral("testuser"), nullptr);
    QTabWidget *tabs = w->findChild<QTabWidget*>();
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 3);
    QCOMPARE(tabs->tabText(0), QStringLiteral("Amigos"));
    QCOMPARE(tabs->tabText(1), QStringLiteral("Solicitudes"));
    QCOMPARE(tabs->tabText(2), QStringLiteral("Buscar"));

    // Cada pestaña contiene su QListWidget o QLineEdit
    QWidget *friendsTab = tabs->widget(0);
    QVERIFY(friendsTab->findChild<QListWidget*>());

    QWidget *reqTab = tabs->widget(1);
    QVERIFY(reqTab->findChild<QListWidget*>());

    QWidget *searchTab = tabs->widget(2);
    QVERIFY(searchTab->findChild<QLineEdit*>());
    QVERIFY(searchTab->findChild<QListWidget*>());

    delete w; w = nullptr;
}

void TestFriendsWindow::test_friend_request_search_tab()
{
    w = new friendswindow(QStringLiteral("testuser"), nullptr);
    QTabWidget *tabs = w->findChild<QTabWidget*>();
    tabs->setCurrentIndex(2); // pestaña "Buscar"
    QTest::qWait(50);

    QLineEdit *edit = w->findChild<QLineEdit*>();
    QVERIFY(edit);
    // Al cambiar texto, debe invocar searchUsers y limpiar la lista
    QListWidget *results = w->findChild<QListWidget*>("", Qt::FindChildrenRecursively);
    QVERIFY(results);
    edit->setText("foo");
    QTest::qWait(100);
    // Como no hay backend en test, la lista seguirá vacía
    QCOMPARE(results->count(), 0);

    delete w; w = nullptr;
}

void TestFriendsWindow::test_close_button_closes()
{
    w = new friendswindow(QStringLiteral("testuser"), nullptr);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));

    QPushButton *btnClose = w->findChild<QPushButton*>();
    QVERIFY(btnClose);
    QTest::mouseClick(btnClose, Qt::LeftButton);
    QTRY_VERIFY(!w->isVisible());

    delete w; w = nullptr;
}
