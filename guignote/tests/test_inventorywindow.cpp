#include "test_inventorywindow.h"
#include "inventorywindow.h"

#include <QtTest/QtTest>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>

void TestInventoryWindow::initTestCase()
{
    // Nada especial antes de todos los tests
}

void TestInventoryWindow::cleanupTestCase()
{
    // Nada especial al final
}

void TestInventoryWindow::test_window_flags_and_size()
{
    w = new InventoryWindow(nullptr);
    // Debe ser sin marco y diálogo
    QVERIFY(w->windowFlags() & Qt::FramelessWindowHint);
    QCOMPARE(w->size(), QSize(800,600));
    delete w;
    w = nullptr;
}

void TestInventoryWindow::test_basic_widgets_exist()
{
    w = new InventoryWindow(nullptr);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));

    // 1) Botón de cierre en la esquina superior
    QPushButton *btnClose = w->findChild<QPushButton*>();
    QVERIFY(btnClose);
    QCOMPARE(btnClose->iconSize(), QSize(18,18));
    QCOMPARE(btnClose->size(), QSize(30,30));

    // 2) Sidebar con dos ítems
    QListWidget *sidebar = w->findChild<QListWidget*>();
    QVERIFY(sidebar);
    QCOMPARE(sidebar->count(), 2);
    QCOMPARE(sidebar->item(0)->text(), QStringLiteral("Barajas"));
    QCOMPARE(sidebar->item(1)->text(), QStringLiteral("Tapetes"));

    // 3) StackedWidget con dos páginas
    QStackedWidget *stack = w->findChild<QStackedWidget*>();
    QVERIFY(stack);
    QCOMPARE(stack->count(), 2);

    // 4) Cada página tiene un QLabel de título
    QWidget *page0 = stack->widget(0);
    QLabel *lbl0 = page0->findChild<QLabel*>();
    QVERIFY(lbl0);
    QCOMPARE(lbl0->text(), QStringLiteral("Gestión de Barajas"));

    QWidget *page1 = stack->widget(1);
    QLabel *lbl1 = page1->findChild<QLabel*>();
    QVERIFY(lbl1);
    QCOMPARE(lbl1->text(), QStringLiteral("Gestión de Tapetes"));

    delete w;
    w = nullptr;
}

void TestInventoryWindow::test_close_button_closes()
{
    w = new InventoryWindow(nullptr);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));

    QPushButton *btnClose = w->findChild<QPushButton*>();
    QVERIFY(btnClose);
    QTest::mouseClick(btnClose, Qt::LeftButton);
    QTRY_VERIFY(!w->isVisible());

    delete w;
    w = nullptr;
}

void TestInventoryWindow::test_sidebar_pages()
{
    w = new InventoryWindow(nullptr);
    QListWidget *sidebar = w->findChild<QListWidget*>();
    QStackedWidget *stack = w->findChild<QStackedWidget*>();
    QVERIFY(sidebar && stack);

    // Inicialmente, índice 0
    QCOMPARE(stack->currentIndex(), 0);

    // Cambiar al segundo ítem pone página 1
    sidebar->setCurrentRow(1);
    QTest::qWait(50);
    QCOMPARE(stack->currentIndex(), 1);

    // Volver al primero
    sidebar->setCurrentRow(0);
    QTest::qWait(50);
    QCOMPARE(stack->currentIndex(), 0);

    delete w;
    w = nullptr;
}

void TestInventoryWindow::test_switching_pages_updates_stack()
{
    w = new InventoryWindow(nullptr);
    QListWidget *sidebar = w->findChild<QListWidget*>();
    QStackedWidget *stack = w->findChild<QStackedWidget*>();
    QVERIFY(sidebar && stack);

    // Recorremos todas las filas del sidebar y comprobamos stack
    for (int row=0; row<sidebar->count(); ++row) {
        sidebar->setCurrentRow(row);
        QTest::qWait(20);
        QCOMPARE(stack->currentIndex(), row);
    }

    delete w;
    w = nullptr;
}
