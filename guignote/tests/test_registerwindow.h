#pragma once
#include <QObject>

class TestRegisterWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_fields_exist();
    void test_toggle_password_visibility();
    void test_toggle_confirm_visibility();
    void test_password_mismatch_shows_error();
    void test_back_button_closes();
    void test_login_link_emits_signal();

    // Tests exhaustivos adicionales
    void test_long_input_length();
    void test_rapid_toggle_password();
    void test_rapid_toggle_confirm();
    void test_multiple_register_clicks();
    void test_special_characters_input();
    void test_placeholder_texts();
    void test_styles_applied();
};
