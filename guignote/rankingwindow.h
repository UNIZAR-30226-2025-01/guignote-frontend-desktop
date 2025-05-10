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
    explicit RankingWindow(const QString &userKey, QWidget *parent = nullptr);

private:
    QString currentUserName;
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
    QString loadAuthToken(const QString &userKey);

    void fetchIndividualRanking();
    void fetchTeamRanking();
    void handleIndividualRankingResponse();
    void handleTeamRankingResponse();
    void updateRankingList(const QJsonArray &playersArray, int type);

    QString amigos = "";
    int lastPressed = 1;
};

#endif // RANKINGWINDOW_H
