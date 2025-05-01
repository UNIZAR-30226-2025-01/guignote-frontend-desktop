#include "test_registerwindow.h"

#include <QtTest/QtTest>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QAction>
#include <QTimer>

#include "registerwindow.h"

void TestRegisterWindow::initTestCase() {}
void TestRegisterWindow::cleanupTestCase() {}

void TestRegisterWindow::test_fields_exist()
{
    RegisterWindow w;
    w.show();

    auto edits = w.findChildren<QLineEdit*>();
    QVERIFY(edits.size() >= 4);

    bool foundCreate = false, foundBack = false;
    for (auto *btn : w.findChildren<QPushButton*>()) {
        if (btn->text() == "Crear Cuenta") foundCreate = true;
        if (btn->text() == "Volver")        foundBack = true;
    }
    QVERIFY(foundCreate);
    QVERIFY(foundBack);
}

void TestRegisterWindow::test_toggle_password_visibility()
{
    RegisterWindow w;
    w.show();

    QLineEdit *pwdEdit = nullptr;
    for (auto *e : w.findChildren<QLineEdit*>())
        if (e->echoMode() == QLineEdit::Password) { pwdEdit = e; break; }
    QVERIFY2(pwdEdit, "No se encontró el campo de contraseña");

    QAction *toggle = pwdEdit->actions().last();
    QVERIFY(toggle);

    QVERIFY(pwdEdit->echoMode() == QLineEdit::Password);
    toggle->trigger();
    QVERIFY(pwdEdit->echoMode() == QLineEdit::Normal);
    toggle->trigger();
    QVERIFY(pwdEdit->echoMode() == QLineEdit::Password);
}

void TestRegisterWindow::test_toggle_confirm_visibility()
{
    RegisterWindow w;
    w.show();

    QList<QLineEdit*> pwds;
    for (auto *e : w.findChildren<QLineEdit*>())
        if (e->echoMode() == QLineEdit::Password) pwds.append(e);
    QVERIFY2(pwds.size() >= 2, "No se encontraron dos campos de contraseña");

    QLineEdit *confirmEdit = pwds.at(1);
    QAction *toggle = confirmEdit->actions().last();
    QVERIFY(toggle);

    QVERIFY(confirmEdit->echoMode() == QLineEdit::Password);
    toggle->trigger();
    QVERIFY(confirmEdit->echoMode() == QLineEdit::Normal);
    toggle->trigger();
    QVERIFY(confirmEdit->echoMode() == QLineEdit::Password);
}

void TestRegisterWindow::test_password_mismatch_shows_error()
{
    RegisterWindow w;
    w.show();

    auto edits = w.findChildren<QLineEdit*>();
    edits.at(0)->setText("user");
    edits.at(1)->setText("user@example.com");
    edits.at(2)->setText("abc");
    edits.at(3)->setText("def");

    bool sawError = false;
    QTimer::singleShot(0, [&](){
        for (auto *top : QApplication::topLevelWidgets()) {
            if (auto *msg = qobject_cast<QMessageBox*>(top)) {
                sawError = true;
                msg->accept();
            }
        }
    });

    for (auto *btn : w.findChildren<QPushButton*>()) {
        if (btn->text() == "Crear Cuenta") {
            QTest::mouseClick(btn, Qt::LeftButton);
            break;
        }
    }

    QTest::qWait(100);
    QVERIFY2(sawError, "No se mostró QMessageBox de contraseñas no coincidentes");
}

void TestRegisterWindow::test_back_button_closes()
{
    RegisterWindow w;
    w.show();

    QTimer::singleShot(50, [&](){
        for (auto *btn : w.findChildren<QPushButton*>()) {
            if (btn->text() == "Volver") {
                QTest::mouseClick(btn, Qt::LeftButton);
                return;
            }
        }
    });

    bool closed = false;
    connect(&w, &QDialog::finished, [&](){ closed = true; });
    w.exec();
    QVERIFY2(closed, "La ventana de registro no se cerró al pulsar Volver");
}

void TestRegisterWindow::test_login_link_emits_signal()
{
    RegisterWindow w;
    w.show();

    QSignalSpy spy(&w, &RegisterWindow::openLoginRequested);
    QCOMPARE(spy.count(), 0);

    for (auto *btn: w.findChildren<QPushButton*>()) {
        if (btn->text().contains("¿Ya tienes cuenta")) {
            QTest::mouseClick(btn, Qt::LeftButton);
            break;
        }
    }

    QTest::qWait(50);
    QCOMPARE(spy.count(), 1);
}

void TestRegisterWindow::test_long_input_length()
{
    RegisterWindow w;
    w.show();

    auto edits = w.findChildren<QLineEdit*>();
    QString longStr(5000, QChar('x'));
    for (auto *e : edits) {
        e->clear();
        e->setText(longStr);
        QCOMPARE(e->text().length(), 5000);
    }
}

void TestRegisterWindow::test_rapid_toggle_password()
{
    RegisterWindow w;
    w.show();

    QLineEdit *pwdEdit = nullptr;
    for (auto *e : w.findChildren<QLineEdit*>())
        if (e->echoMode() == QLineEdit::Password) { pwdEdit = e; break; }
    QVERIFY(pwdEdit);

    QAction *toggle = pwdEdit->actions().last();
    QVERIFY(toggle);

    for (int i = 0; i < 100; ++i) {
        toggle->trigger();
        QTest::qWait(1);
    }
}

void TestRegisterWindow::test_rapid_toggle_confirm()
{
    RegisterWindow w;
    w.show();

    QList<QLineEdit*> pwds;
    for (auto *e : w.findChildren<QLineEdit*>())
        if (e->echoMode() == QLineEdit::Password) pwds.append(e);
    QVERIFY(pwds.size() >= 2);

    QAction *toggle = pwds.at(1)->actions().last();
    QVERIFY(toggle);

    for (int i = 0; i < 100; ++i) {
        toggle->trigger();
        QTest::qWait(1);
    }
}

void TestRegisterWindow::test_multiple_register_clicks()
{
    RegisterWindow w;
    w.show();

    auto edits = w.findChildren<QLineEdit*>();
    edits.at(0)->setText("user");
    edits.at(1)->setText("a@b.com");
    edits.at(2)->setText("password");
    edits.at(3)->setText("password");

    QPushButton *btn = nullptr;
    for (auto *b : w.findChildren<QPushButton*>())
        if (b->text() == "Crear Cuenta") { btn = b; break; }
    QVERIFY(btn);

    for (int i = 0; i < 20; ++i)
        QTest::mouseClick(btn, Qt::LeftButton);

    QTest::qWait(50);
    QVERIFY(true);
}

void TestRegisterWindow::test_special_characters_input()
{
    RegisterWindow w;
    w.show();

    QStringList weird = {
        "àéîöû",
        "漢字仮名交じり文",
        "😀🐍🚀",
        "<script>alert('x')</script>",
        "' OR 1=1; --",
        "DROP TABLE users;",
        QByteArray::fromHex("f09f9880").constData()
    };

    auto edits = w.findChildren<QLineEdit*>();
    for (const QString &text : weird) {
        for (auto *e : edits) {
            e->clear();
            e->setText(text);
            QCOMPARE(e->text(), text);
        }
    }
}

void TestRegisterWindow::test_placeholder_texts()
{
    RegisterWindow w;
    w.show();

    QStringList expected = {
        "Usuario",
        "Correo Electrónico",
        "Contraseña",
        "Confirmar Contraseña"
    };
    auto edits = w.findChildren<QLineEdit*>();
    QVERIFY2(edits.size() >= expected.size(), "Número de campos insuficiente");
    for (int i = 0; i < expected.size(); ++i) {
        QCOMPARE(edits.at(i)->placeholderText(), expected.at(i));
    }
}

void TestRegisterWindow::test_styles_applied()
{
    RegisterWindow w;
    w.show();

    QPushButton *registerBtn = nullptr;
    for (auto *b : w.findChildren<QPushButton*>())
        if (b->text() == "Crear Cuenta") { registerBtn = b; break; }
    QVERIFY(registerBtn);

    QString style = registerBtn->styleSheet();
    QVERIFY(style.contains("background-color: #c2c2c3"));
    QVERIFY(style.contains("font-size: 20px"));
}
