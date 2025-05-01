#ifndef TEST_SETTINGSWINDOW_H
#define TEST_SETTINGSWINDOW_H

#include <QtTest/QtTest>

// forward-declarations
class SettingsWindow;

/* ──────────────────────────────────────────────── *
 *  Test suite para SettingsWindow                  *
 * ──────────────────────────────────────────────── */
class TestSettingsWindow : public QObject
{
    Q_OBJECT
private slots:
    /* fixture */
    void init();       // se ejecuta antes de cada test
    void cleanup();    // se ejecuta después de cada test

    /* casos */
    void test_basic_widgets_exist();
    void test_close_button_closes();
    void test_slider_emits_volume();
    void test_save_and_load_settings();
private:
    SettingsWindow *w {nullptr};
};

#endif // TEST_SETTINGSWINDOW_H
