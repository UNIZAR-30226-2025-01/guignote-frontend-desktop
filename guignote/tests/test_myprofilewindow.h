#ifndef TEST_MYPROFILEWINDOW_H
#define TEST_MYPROFILEWINDOW_H

#include <QObject>
#include <QSize>

class TestMyProfileWindow : public QObject
{
    Q_OBJECT
public:
    explicit TestMyProfileWindow(QObject *parent = nullptr);
private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_basic_widgets_exist();
    void test_window_flags_and_size();
    void test_close_button_closes();
    void test_header_layout();
    void test_profile_picture_clickable();
    void test_labels_initial_text();
    void test_logout_button_closes_and_resets();

private:
    static const QString kUserKey;
};

#endif // TEST_MYPROFILEWINDOW_H
