// rankswindow.h
#pragma once

#include <QDialog>
#include <QNetworkAccessManager>

class RangeBarWidget;

class RanksWindow : public QDialog {
    Q_OBJECT

public:
    explicit RanksWindow(const QString &userKey, QWidget *parent = nullptr);

private:
    void setupUI();
    QString loadAuthToken();
    void fetchElo();

    QString userKey;
    QNetworkAccessManager *networkManager;
    RangeBarWidget *barWidget;
};
