#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QCloseEvent>
#include <QJsonObject>

class friendswindow : public QDialog {
    Q_OBJECT
public:
    explicit friendswindow(const QString &userKey, QWidget *parent = nullptr);

public slots:
    void removeFriend(const QString &friendId, const QString &userKey);

private:
    // Layout principal y componentes del encabezado
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QTabWidget *tabWidget;

    // Pestaña "Amigos"
    QListWidget *friendsListWidget;

    // Pestaña "Solicitudes"
    QListWidget *requestsListWidget;
    QPushButton *acceptButton;
    QPushButton *rejectButton;

    // Pestaña "Buscar"
    QLineEdit *searchLineEdit;
    QPushButton *searchButton;
    QListWidget *searchResultsListWidget;

    QString currentSearchQuery;

    // Gestor de red para realizar peticiones HTTP
    QNetworkAccessManager *networkManager;

    // Métodos para configurar la interfaz
    void setupUI(const QString &userKey);
    QWidget* createFriendsTab();
    QWidget* createRequestsTab();
    QWidget* createSearchTab(const QString &userKey);

    // Métodos para crear widgets personalizados en las pestañas
    QWidget* createSearchResultWidget(const QJsonObject &usuario, const QString userKey);
    QWidget* createRequestWidget(const QJsonObject &solicitud, const QString userKey);
    QWidget* createFriendWidget(const QJsonObject &amigo, const QString &userKey);

    // Métodos para la conexión con el backend
    QString loadAuthToken(const QString &userKey);
    void fetchFriends(const QString &userKey);
    void fetchRequests(const QString &userKey);
    void searchUsers(const QString &userKey);
    void sendFriendRequest(const QString &userKey);
    void acceptRequest(const QString &userKey);
    void rejectRequest(const QString &userKey);
};

#endif // FRIENDSWINDOW_H
