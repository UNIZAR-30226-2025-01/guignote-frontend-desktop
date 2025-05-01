#ifndef TEST_FRIENDSWINDOW_H
#define TEST_FRIENDSWINDOW_H

#include <QObject>

class friendswindow;

class TestFriendsWindow : public QObject
{
    Q_OBJECT

private:
    friendswindow *w = nullptr;

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_window_flags_and_size();
    void test_basic_ui_elements();
    void test_tab_count_and_titles();
    void test_friend_request_search_tab();
    void test_close_button_closes();
};
#endif // TEST_FRIENDSWINDOW_H
