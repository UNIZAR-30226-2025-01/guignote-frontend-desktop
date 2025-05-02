#include "loadingwindow.h"
#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QUrl>
// Función para hacer login de forma sincrónica
bool tryLogin(const QString &user, const QString &pass, QString &outToken) {
    QUrl url("http://188.165.76.134:8000/usuarios/iniciar_sesion/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    if (user.contains('@'))
        json["correo"] = user;
    else
        json["nombre"] = user;
    json["contrasegna"] = pass;
    QJsonDocument doc(json);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, doc.toJson());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        if (responseDoc.isObject()) {
            QJsonObject respObj = responseDoc.object();
            if (respObj.contains("token")) {
                outToken = respObj["token"].toString();
                reply->deleteLater();
                return true;
            }
        }
    }
    reply->deleteLater();
    return false;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Al salir de la aplicación, si "Recordar" no está marcado,
    // eliminamos las credenciales (usuario y contraseña)
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [](){
        QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
        if (!settings.value("auth/remember", true).toBool()) {
            settings.remove("auth/user");
            settings.remove("auth/pass");
        }
    });

    // Leer credenciales guardadas
    QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
    QString user = settings.value("auth/user", "").toString();
    QString pass = settings.value("auth/pass", "").toString();

    // 1) Si no hay credenciales → flujo normal
    if (user.isEmpty() || pass.isEmpty()) {
        MainWindow w;
        w.show();
        return a.exec();
    }

    // 2) Si hay credenciales, intento login automático
    QString token;
    if (tryLogin(user, pass, token)) {
        QSettings userSettings("Grace Hopper", QString("Sota, Caballo y Rey_%1").arg(user));
        userSettings.setValue("auth/token", token);
        // Muestro directamente el LoadingWindow
        LoadingWindow *l = new LoadingWindow(user);
        l->show();
        return a.exec();
    } else {
        // Si falla, vuelvo al login manual
        MainWindow w;
        w.show();
        return a.exec();
    }

}
