#ifndef REJOINWINDOW_H
#define REJOINWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QIcon>
#include <QJsonArray>

class RejoinWindow : public QDialog {
    Q_OBJECT

public:
    explicit RejoinWindow(QJsonArray jsonArray, QWidget *parent = nullptr);

private:
    void setupUI();
    QHBoxLayout* createHeaderLayout();
    void populateSalas();

    QJsonArray salas;

    QPushButton *closeButton;
    QLabel *titleLabel;
    QVBoxLayout *mainLayout;
};

#endif // REJOINWINDOW_H
