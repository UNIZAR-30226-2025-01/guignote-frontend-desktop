#include "test_mainwindow.h"

#include <QtTest>
#include <QPushButton>
#include <QTimer>

#include "mainwindow.h"
#include "loginwindow.h"

/* ---------- test 1 ---------- */
void TestMainWindow::test_botones_existen()
{
    MainWindow w;  w.show();

    bool tieneLogin = false, tieneRegister = false;
    for (auto *b : w.findChildren<QPushButton*>()) {
        if (b->text() == "Iniciar Sesión") tieneLogin    = true;
        if (b->text() == "Crear Cuenta")   tieneRegister = true;
    }
    QVERIFY(tieneLogin);
    QVERIFY(tieneRegister);
}

/* ---------- test 2 ---------- */
void TestMainWindow::test_abre_login()
{
    MainWindow w;
    w.show();

    // 1) Programamos el clic
    QTimer::singleShot(0, [&](){
        // Buscamos el botón "Iniciar Sesión"
        for (auto *btn : w.findChildren<QPushButton*>()) {
            if (btn->text() == "Iniciar Sesión") {
                QTest::mouseClick(btn, Qt::LeftButton);
                return;
            }
        }
    });

    // Flag para saber si el diálogo llegó a verse
    bool loginVisible = false;

    // 2) Programamos el cierre del diálogo y marcamos el flag
    QTimer::singleShot(100, [&](){
        for (auto *top : QApplication::topLevelWidgets()) {
            if (auto *dlg = qobject_cast<LoginWindow*>(top); dlg && dlg->isVisible()) {
                loginVisible = true;
                dlg->done(QDialog::Accepted);
                return;
            }
        }
    });

    // 3) Dejamos que todo ocurra
    QTest::qWait(200);

    // Ahora comprobamos
    QVERIFY2(loginVisible, "LoginWindow no se mostró tras el click");
}




