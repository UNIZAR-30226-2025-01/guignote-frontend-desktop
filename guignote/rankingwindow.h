#ifndef RANKINGWINDOW_H
#define RANKINGWINDOW_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

class RankingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RankingWindow(QWidget *parent = nullptr);

private:
    QPushButton *individualButton;
    QPushButton *parejasButton;
    QCheckBox *soloAmigosCheck;

    void setupUI();

    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QNetworkAccessManager *networkManager;
};

#endif // RANKINGWINDOW_H

