#ifndef TEST_RANKINGWINDOW_H
#define TEST_RANKINGWINDOW_H

#include <QObject>

class TestRankingWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_basic_widgets_exist();
    void test_close_button_closes();
    void test_update_ranking_list_individual();
    void test_update_ranking_list_parejas();
};

#endif // TEST_RANKINGWINDOW_H
