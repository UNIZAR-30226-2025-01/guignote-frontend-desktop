#pragma once
#include <QtTest/QtTest>
#include "userprofilewindow.h"

/*
 *  Suite de pruebas para UserProfileWindow
 */
class TestUserProfileWindow : public QObject
{
    Q_OBJECT
private:
    UserProfileWindow *w = nullptr;

private slots:
    /* fixture */
    void init();        // se ejecuta antes de cada prueba
    void cleanup();     // se ejecuta después de cada prueba

    /* casos de prueba */
    void test_basic_widgets_exist();
    void test_window_flags_and_size();
    void test_close_button_closes();
    void test_circular_pixmap_shape();
    void test_bottom_buttons_clickable();
};
