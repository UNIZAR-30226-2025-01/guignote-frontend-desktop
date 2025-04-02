#ifndef RANKINGWINDOW_H
#define RANKINGWINDOW_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QJsonArray>

class RankingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RankingWindow(QWidget *parent = nullptr);

private:
    // UI
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QPushButton *individualButton;
    QPushButton *parejasButton;
    QCheckBox *soloAmigosCheck;
    QListWidget *rankingListWidget;

    // Networking
    QNetworkAccessManager *networkManager;
    QString authToken;

    // Métodos
    void setupUI();
    QString loadAuthToken();

    void fetchIndividualRanking();
    void fetchTeamRanking();
    void handleIndividualRankingResponse();
    void handleTeamRankingResponse();
    void updateRankingList(const QJsonArray &playersArray);
};

#endif // RANKINGWINDOW_H
