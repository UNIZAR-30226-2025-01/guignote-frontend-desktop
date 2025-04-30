#ifndef TEST_MENUWINDOW_H
#define TEST_MENUWINDOW_H

#include <QObject>
#include <QtTest/QtTest>

/* Declaración adelantada para usar MenuWindow como puntero */
class MenuWindow;

/**
 * Pruebas unitarias (sólo con API pública) para MenuWindow.
 */
class TestMenuWindow : public QObject
{
    Q_OBJECT
private slots:
    /* Fixture ------------------------------------------------------- */
    void init();
    void cleanup();

    /* ---------- casos básicos ---------- */
    void test_basic_widgets_exist();
    void test_play_buttons_open_dialog();
    void test_cancel_button_closes_dialog();
    void test_spam_protection_one_dialog();
    void test_icons_open_some_modal();
    void test_resize_repositions_elements();

    /* ---------- stress / edge cases ---------- */
    void test_extreme_window_sizes();
    void test_multiple_cancel_clicks();
    void test_search_dialog_show_hide_rapidly();

    void cleanupTestCase();

private:
    MenuWindow *w = nullptr;
};

#endif // TEST_MENUWINDOW_H
