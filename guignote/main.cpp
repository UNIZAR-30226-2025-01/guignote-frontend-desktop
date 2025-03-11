#include "mainwindow.h"
#include "menuwindow.h"
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

    QSettings settings("Grace Hopper", "Sota, Caballo y Rey");
    QString user = settings.value("auth/user", "").toString();
    QString pass = settings.value("auth/pass", "").toString();

    // Si no hay credenciales, mostramos la pantalla de inicio de sesión
    if (user.isEmpty() || pass.isEmpty()) {
        MainWindow w;
        w.show();
        return a.exec();
    }

    // Si hay credenciales, intentamos hacer login
    QString token;
    bool ok = tryLogin(user, pass, token);
    if (ok) {
        // Guardamos el token para usarlo en las peticiones si lo necesitas
        settings.setValue("auth/token", token);
        MenuWindow w;
        w.show();
        return a.exec();
    } else {
        MainWindow w;
        w.show();
        return a.exec();
    }
*/
}
