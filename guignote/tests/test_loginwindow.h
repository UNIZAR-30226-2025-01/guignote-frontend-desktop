#pragma once
#include <QObject>

class TestLoginWindow : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()  {}
    void cleanupTestCase(){}

    void test_fields_exist();
    void test_toggle_password_visibility();
    void test_empty_fields_shows_error();
    void test_open_register_link_emits_signal();

    /* pruebas exhaustivas añadidas */
    void test_special_characters_input();
    void test_long_input_length();
    void test_multiple_login_clicks_spam();
};
