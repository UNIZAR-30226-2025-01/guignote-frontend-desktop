#pragma once
#include <QObject>

class TestMainWindow : public QObject
{
    Q_OBJECT
private slots:
    void test_botones_existen();
    void test_abre_login();
};
