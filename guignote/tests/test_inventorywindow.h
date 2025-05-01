#ifndef TEST_INVENTORYWINDOW_H
#define TEST_INVENTORYWINDOW_H

#include <QObject>

class InventoryWindow;

class TestInventoryWindow : public QObject
{
    Q_OBJECT

private:
    InventoryWindow *w = nullptr;

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_window_flags_and_size();
    void test_basic_widgets_exist();
    void test_close_button_closes();
    void test_sidebar_pages();
    void test_switching_pages_updates_stack();
};

#endif // TEST_INVENTORYWINDOW_H
