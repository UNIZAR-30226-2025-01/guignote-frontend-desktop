#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>

class friendswindow : public QDialog {
    Q_OBJECT

public:
    explicit friendswindow(QWidget *parent = nullptr);

private:
    QPushButton *closeButton;
    QLabel *titleLabel;
    QLineEdit *searchBar;
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;

    void setupUI();  // Configura la UI
    QWidget* createFriendsTab();   // Crea la pestaña de amigos
    QWidget* createRequestsTab();  // Crea la pestaña de solicitudes de amistad
};

#endif // FRIENDSWINDOW_H
