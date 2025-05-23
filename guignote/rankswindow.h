// rankswindow.h
#pragma once

#include <QDialog>
#include <QNetworkAccessManager>
#include <QLabel>

class RangeBarWidget;

class RanksWindow : public QDialog {
    Q_OBJECT

public:
    explicit RanksWindow(const QString &userKey, QWidget *parent = nullptr);

private:
    void setupUI();
    QString loadAuthToken();
    void fetchElo();

    QLabel* eloLabel = nullptr;

    QString userKey;
    QNetworkAccessManager *networkManager;
    RangeBarWidget *barWidget;
    QLabel* rangoIconLabel;
    QLabel* rankIconLabel;
    QVector<QString> m_icons;
    QVector<QString> m_rangos;
    QVector<int>     m_thresholds;
};
