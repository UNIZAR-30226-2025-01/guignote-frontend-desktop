#include "test_rankingwindow.h"

#include <QtTest/QtTest>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonObject>

// ------------------------------------------------------------------
// Aquí redefinimos private como public **antes** de incluir el header
// ------------------------------------------------------------------
#define private public
#include "rankingwindow.h"
#undef private
// ------------------------------------------------------------------

static const QString kUserKey = QStringLiteral("testuser");

void TestRankingWindow::initTestCase()
{
    // No hay preparación global
}

void TestRankingWindow::cleanupTestCase()
{
    // No hay limpieza global
}

void TestRankingWindow::test_basic_widgets_exist()
{
    RankingWindow w(kUserKey, nullptr);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    // -- Título --
    QLabel *title = w.findChild<QLabel*>();
    QVERIFY(title);
    QCOMPARE(title->text(), QStringLiteral("Rankings"));

    // -- Botones Individual / Parejas --
    const auto buttons = w.findChildren<QPushButton*>();
    bool foundInd = std::any_of(buttons.begin(), buttons.end(),
                                [](QPushButton *b){ return b->text() == QLatin1String("Individual"); });
    bool foundPar = std::any_of(buttons.begin(), buttons.end(),
                                [](QPushButton *b){ return b->text() == QLatin1String("Parejas"); });
    QVERIFY(foundInd);
    QVERIFY(foundPar);

    // -- Checkbox Solo amigos --
    QCheckBox *chk = w.findChild<QCheckBox*>();
    QVERIFY(chk);
    QCOMPARE(chk->text(), QStringLiteral("Solo amigos"));

    // -- Lista de rankings --
    QListWidget *list = w.findChild<QListWidget*>();
    QVERIFY(list);
    QVERIFY(list->model());
}

void TestRankingWindow::test_close_button_closes()
{
    RankingWindow w(kUserKey, nullptr);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    // Pulsar la cruz de cierre
    QPushButton *closeBtn = w.findChild<QPushButton*>();
    QVERIFY(closeBtn);
    QTest::mouseClick(closeBtn, Qt::LeftButton);
    QTRY_VERIFY(!w.isVisible());
}

void TestRankingWindow::test_update_ranking_list_individual()
{
    RankingWindow w(kUserKey, nullptr);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    // Montamos un JSON con dos jugadores individuales
    QJsonArray arr;
    QJsonObject p1; p1["nombre"] = "Alice"; p1["elo"] = 1500; arr.append(p1);
    QJsonObject p2; p2["nombre"] = "Bob";   p2["elo"] = 1400; arr.append(p2);

    // Gracias al hack, esto ya compila
    w.updateRankingList(arr, /*type=*/1);

    // Verificamos
    QListWidget *list = w.findChild<QListWidget*>();
    QCOMPARE(list->count(), 2);
    QVERIFY(list->item(0)->text().contains("Alice"));
    QVERIFY(list->item(0)->text().contains("1500"));
    QVERIFY(list->item(1)->text().contains("Bob"));
    QVERIFY(list->item(1)->text().contains("1400"));
}

void TestRankingWindow::test_update_ranking_list_parejas()
{
    RankingWindow w(kUserKey, nullptr);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    // Montamos un JSON con dos parejas
    QJsonArray arr;
    QJsonObject p1; p1["nombre"] = "Carol"; p1["elo_parejas"] = 1300; arr.append(p1);
    QJsonObject p2; p2["nombre"] = "Dave";  p2["elo_parejas"] = 1200; arr.append(p2);

    w.updateRankingList(arr, /*type=*/2);

    // Verificamos
    QListWidget *list = w.findChild<QListWidget*>();
    QCOMPARE(list->count(), 2);
    QVERIFY(list->item(0)->text().contains("Carol"));
    QVERIFY(list->item(0)->text().contains("1300"));
    QVERIFY(list->item(1)->text().contains("Dave"));
    QVERIFY(list->item(1)->text().contains("1200"));
}
