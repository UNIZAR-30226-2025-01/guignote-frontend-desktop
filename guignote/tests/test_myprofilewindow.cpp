#include "test_myprofilewindow.h"
#include "myprofilewindow.h"
#include "icon.h"

#include <QtTest/QtTest>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QHBoxLayout>

// Definición de la clave de usuario de prueba
const QString TestMyProfileWindow::kUserKey = QStringLiteral("testuser");

// Devuelve la ruta al fichero .conf que usa QSettings("Grace Hopper","Sota, Caballo y Rey")
// ---------- test_myprofilewindow.cpp ------------
static QString configFilePath(const QString &userKey)
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    + QString("/Grace Hopper/Sota, Caballo y Rey_%1.conf").arg(userKey);
}


TestMyProfileWindow::TestMyProfileWindow(QObject *parent)
    : QObject(parent)
{}

void TestMyProfileWindow::initTestCase()
{
    // Limpiar cualquier resto de ejecuciones anteriores
    QSettings s(configFilePath(kUserKey), QSettings::IniFormat);

    s.clear();
}

void TestMyProfileWindow::cleanupTestCase()
{
    // Borrar al finalizar
    QSettings s(configFilePath(kUserKey), QSettings::IniFormat);
    s.clear();
}

void TestMyProfileWindow::test_basic_widgets_exist()
{
    MyProfileWindow w(kUserKey, nullptr);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    // --- 1) Título EXACTO ---
    // En MyProfileWindow::createHeaderLayout() haces:
    //     titleLabel = new QLabel("Perfil", this);
    //
    // Así que aquí lo buscamos directamente:
    QLabel *title = w.findChild<QLabel*>("", Qt::FindDirectChildrenOnly);
    QVERIFY(title);
    QCOMPARE(title->text(), QStringLiteral("Perfil"));

    // --- 2) Botón de cierre ---
    QPushButton *btnClose = w.findChild<QPushButton*>();
    QVERIFY(btnClose);

    // --- 3) Foto de perfil (Icon) sin hoverEnabled ---
    Icon *foto = w.findChild<Icon*>();
    QVERIFY(foto);
    QVERIFY(!foto->testAttribute(Qt::WA_Hover));

    // --- 4) Labels de usuario y estadísticas ---
    QLabel *userLabel = nullptr;
    QLabel *statsLabel = nullptr;
    for (auto *lbl : w.findChildren<QLabel*>()) {
        if (lbl == title) continue;
        if (lbl->text().contains("Usuario"))      userLabel = lbl;
        else if (lbl->text().contains("Victorias")) statsLabel = lbl;
    }
    QVERIFY(userLabel);
    QVERIFY(statsLabel);
}


void TestMyProfileWindow::test_window_flags_and_size()
{
    MyProfileWindow w(kUserKey, nullptr);
    QVERIFY(w.windowFlags() & Qt::FramelessWindowHint);
    QCOMPARE(w.size(), QSize(850, 680));
}

void TestMyProfileWindow::test_close_button_closes()
{
    MyProfileWindow w(kUserKey, nullptr);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    auto *btn = w.findChild<QPushButton*>();
    QVERIFY(btn);
    QTest::mouseClick(btn, Qt::LeftButton);
    QTRY_VERIFY(!w.isVisible());
}

void TestMyProfileWindow::test_header_layout()
{
    MyProfileWindow w(kUserKey, nullptr);
    auto layouts = w.findChildren<QHBoxLayout*>();
    QVERIFY(!layouts.isEmpty());
    QVERIFY(layouts.first()->count() >= 2);
}

void TestMyProfileWindow::test_profile_picture_clickable()
{
    MyProfileWindow w(kUserKey, nullptr);
    Icon *foto = w.findChild<Icon*>();
    QVERIFY(foto);

    QSignalSpy spy(foto, &Icon::clicked);
    QVERIFY(spy.isValid());

    QTest::mouseClick(foto, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void TestMyProfileWindow::test_labels_initial_text()
{
    MyProfileWindow w(kUserKey, nullptr);

    QLabel *userLabel = nullptr, *statsLabel = nullptr;
    for (auto *lbl : w.findChildren<QLabel*>()) {
        if (lbl->text().contains("Usuario"))   userLabel = lbl;
        else if (lbl->text().contains("Victorias")) statsLabel = lbl;
    }
    QVERIFY(userLabel);
    QVERIFY(statsLabel);
    QVERIFY(userLabel->text().contains("(0)"));
    QVERIFY(statsLabel->text().contains("Victorias: 0"));
}

