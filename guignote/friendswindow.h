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
    explicit friendswindow(QWidget *parent = nullptr);


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

    // Gestor de red para realizar peticiones HTTP
    QNetworkAccessManager *networkManager;

    // Métodos para configurar la interfaz
    void setupUI();
    QWidget* createFriendsTab();
    QWidget* createRequestsTab();
    QWidget* createSearchTab();

    // Métodos para crear widgets personalizados en las pestañas
    QWidget* createSearchResultWidget(const QJsonObject &usuario);
    QWidget* createRequestWidget(const QJsonObject &solicitud);

    // Métodos para la conexión con el backend
    QString loadAuthToken();
    void fetchFriends();
    void fetchRequests();
    void searchUsers();
    void sendFriendRequest();
    void acceptRequest();
    void rejectRequest();
};

#endif // FRIENDSWINDOW_H
