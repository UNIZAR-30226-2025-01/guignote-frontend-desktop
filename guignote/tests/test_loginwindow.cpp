#include "test_loginwindow.h"

#include <QtTest/QtTest>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QAction>
#include <QSignalSpy>
#include <QTimer>

#include "loginwindow.h"

/* ---------- utilidades ---------- */
static QPushButton* findButton(LoginWindow &w, const QString& text)
{
    for (auto *b : w.findChildren<QPushButton*>())
        if (b->text() == text) return b;
    return nullptr;
}

/* ---------- tests básicos ---------- */
void TestLoginWindow::test_fields_exist()
{
    LoginWindow w; w.show();
    auto edits = w.findChildren<QLineEdit*>();
    QVERIFY(edits.size() >= 2);
    QCOMPARE(edits.at(0)->placeholderText(), "Usuario o Correo");
    QCOMPARE(edits.at(1)->placeholderText(), "Contraseña");
    QVERIFY(findButton(w,"Iniciar Sesión"));
    QVERIFY(findButton(w,"Volver"));
}

void TestLoginWindow::test_toggle_password_visibility()
{
    LoginWindow w; w.show();
    QLineEdit *pwd = w.findChildren<QLineEdit*>().at(1);
    QVERIFY(pwd->echoMode()==QLineEdit::Password);
    QAction *toggle = pwd->actions().last();
    toggle->trigger();  QVERIFY(pwd->echoMode()==QLineEdit::Normal);
    toggle->trigger();  QVERIFY(pwd->echoMode()==QLineEdit::Password);
}

void TestLoginWindow::test_empty_fields_shows_error()
{
    LoginWindow w; w.show();
    QLabel *err = w.findChild<QLabel*>(QString(), Qt::FindChildrenRecursively);
    QVERIFY(err);
    QTest::mouseClick(findButton(w,"Iniciar Sesión"), Qt::LeftButton);
    QTest::qWait(50);
    QVERIFY(err->isVisible());
}

void TestLoginWindow::test_open_register_link_emits_signal()
{
    LoginWindow w; w.show();
    QSignalSpy spy(&w, &LoginWindow::openRegisterRequested);
    for (auto *b : w.findChildren<QPushButton*>())
        if (b->text().contains("Crea una")) { QTest::mouseClick(b, Qt::LeftButton); break; }
    QTest::qWait(50);
    QCOMPARE(spy.count(), 1);
}

/* ---------- pruebas exhaustivas ---------- */
void TestLoginWindow::test_special_characters_input()
{
    LoginWindow w; w.show();
    QStringList weird = {
        "àéîöû", "漢字仮名交じり文", "😀🐍🚀",
        "<script>alert('x')</script>", "' OR 1=1; --", "DROP TABLE users;"
    };
    auto edits = w.findChildren<QLineEdit*>();
    for (const QString& txt: weird)
        for (auto *e: edits) { e->setText(txt); QCOMPARE(e->text(), txt); }
}

void TestLoginWindow::test_long_input_length()
{
    LoginWindow w; w.show();
    QString longStr(4000, QLatin1Char('x'));
    auto edits = w.findChildren<QLineEdit*>();
    edits.at(0)->setText(longStr);
    edits.at(1)->setText(longStr);
    QCOMPARE(edits.at(0)->text().length(), 4000);
    QCOMPARE(edits.at(1)->text().length(), 4000);
}

void TestLoginWindow::test_multiple_login_clicks_spam()
{
    LoginWindow w; w.show();
    auto edits = w.findChildren<QLineEdit*>();
    edits.at(0)->setText("Julito");
    edits.at(1)->setText("123");
    QPushButton *login = findButton(w,"Iniciar Sesión");
    QVERIFY(login);
    for (int i=0;i<25;++i) QTest::mouseClick(login, Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(true); // no debe crashear
}
