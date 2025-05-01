#include "test_menuwindow.h"

#include "menuwindow.h"
#include "icon.h"          // hace Icon un tipo completo
#include "imagebutton.h"   // idem para ImageButton

#include <QtTest/QtTest>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <algorithm>
#include <QTimer>


using namespace Qt::StringLiterals;

static void autoCloseActiveModal(int delayMs = 150)
{
    QTimer::singleShot(delayMs, []{
        if (QWidget *dlg = QApplication::activeModalWidget()) {
            /* intenta pulsar un botón de cierre si lo hay */
            if (QPushButton *btn = dlg->findChild<QPushButton*>("", Qt::FindChildrenRecursively))
                QMetaObject::invokeMethod(btn, "click", Qt::QueuedConnection);
            else
                dlg->close();          // plan B: cerrar la ventana
        }
    });
}

/*  Cierra todas las ventanas top-level abiertas excepto el menú base  */
/* ------------------------------------------------------------------ */
static void closeAllPopups(MenuWindow *menuUnderTest)
{
    bool again = true;
    while (again) {                         // varias pasadas por si al cerrar
        again = false;                      // se crean/borran widgets
        for (QWidget *ww : QApplication::allWidgets()) {

            if (ww == menuUnderTest)              continue;   // menú que probamos
            if (!ww->isVisible())                 continue;   // ya oculta

            /* Solo nos interesan diálogos / ventanas modales abiertas */
            if (qobject_cast<QDialog*>(ww)) {
                ww->close();                       // pide su cierre
                again = true;                      // vuelvo a recorrer la lista
            }
        }
        /* procesa la cola de eventos para que los cierres se materialicen */
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }
}




/* ------------------------------------------------ fixture -------- */
void TestMenuWindow::init()
{
    w = new MenuWindow(QStringLiteral("testuser"));
    w->show();

    /* Asegurarnos de que la ventana está visible y lista */
    QVERIFY(QTest::qWaitForWindowExposed(w));
}

void TestMenuWindow::cleanup()
{
    delete w;
    w = nullptr;
}

/* ------------------------------------------------ casos ---------- */
void TestMenuWindow::test_basic_widgets_exist()
{
    /* Existen dos botones de tipo ImageButton */
    QCOMPARE(w->findChildren<ImageButton*>().size(), 2);

    /* Barras superior e inferior (objectName de la hoja de estilo) */
    QVERIFY(w->findChild<QFrame*>("topBar"));
    QVERIFY(w->findChild<QFrame*>("bottomBar"));

    /* Cinco iconos  */
    QCOMPARE(w->findChildren<Icon*>().size(), 5);
}

/* ------------------------------------------------------------ */
/* 1. Reemplaza TODA la función test_play_buttons_open_dialog() */
/* ------------------------------------------------------------ */
void TestMenuWindow::test_play_buttons_open_dialog()
{
    ImageButton *btn1v1 = w->findChildren<ImageButton*>().first();
    QTest::mouseClick(btn1v1, Qt::LeftButton);

    // Esperar a que aparezca el diálogo
    QDialog *dlg = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT(
        (dlg = w->findChild<QDialog*>()) != nullptr, 1000);

    // Esperar a que exista un QLabel con "(x/y)"
    QLabel *counter = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT(
        ([&](){
            for (QLabel *lab : dlg->findChildren<QLabel*>())
                if (lab->text().contains('(') && lab->text().contains('/'))
                { counter = lab; return true; }
            return false;
        })(), 1000);

    QVERIFY(counter);            // debe haberse encontrado
    dlg->close();                // limpieza
}


void TestMenuWindow::test_cancel_button_closes_dialog()
{
    ImageButton *btn1v1 = w->findChildren<ImageButton*>().first();
    QTest::mouseClick(btn1v1, Qt::LeftButton);

    QDialog *dlg = w->findChild<QDialog*>();
    QVERIFY(dlg);

    // ← REEMPLAZA TODO ESTE BLOQUE ↓↓↓
    // QPushButton *cancel = std::find_if(
    //     dlg->findChildren<QPushButton*>().cbegin(),
    //     dlg->findChildren<QPushButton*>().cend(),
    //     [](QPushButton *b){ return b->text()=="Cancelar"; }
    // )->operator*();
    // QVERIFY(cancel);
    QPushButton *cancel = nullptr;
    auto buttons = dlg->findChildren<QPushButton*>();
    for (QPushButton *b : buttons) {
        if (b->text() == u"Cancelar"_s) {
            cancel = b;
            break;
        }
    }
    QVERIFY(cancel);
    // ← HASTA AQUÍ ↑↑↑

    QTest::mouseClick(cancel, Qt::LeftButton);
    QTRY_VERIFY(!dlg->isVisible());
}


void TestMenuWindow::test_spam_protection_one_dialog()
{
    ImageButton *btn1v1 = w->findChildren<ImageButton*>().first();
    /* Click dos veces muy rápido: no debe abrirse un segundo diálogo */
    QTest::mouseClick(btn1v1, Qt::LeftButton);
    QTest::mouseClick(btn1v1, Qt::LeftButton);
    QTest::qWait(50);

    QCOMPARE(w->findChildren<QDialog*>().size(), 1);
    w->findChild<QDialog*>()->close();
}

void TestMenuWindow::test_icons_open_some_modal()
{
    for (Icon *ic : w->findChildren<Icon*>()) {
        autoCloseActiveModal();        // ←  NUEVA línea
        /* 1) cuántos diálogos había antes del click */
        int before = 0;
        for (QWidget *tlw : QApplication::topLevelWidgets())
            if (qobject_cast<QDialog*>(tlw)) ++before;

        /* 2) click en el icono */
        QTest::mouseClick(ic, Qt::LeftButton);

        /* 3) esperar como máximo 1 s a que aparezca un nuevo diálogo */
        bool created = false;
        QTRY_VERIFY_WITH_TIMEOUT(
            ([&](){
                int now = 0;
                for (QWidget *tlw : QApplication::topLevelWidgets())
                    if (qobject_cast<QDialog*>(tlw)) ++now;
                created = (now > before);
                return created;
            })(), 1000);

        QVERIFY(created);

        closeAllPopups(w);      //  ←  w es el MenuWindow del fixture

    }
}

void TestMenuWindow::test_resize_repositions_elements()
{
    QRect topBarBefore = w->findChild<QFrame*>("topBar")->geometry();

    QSize newSize(1600, 900);
    w->resize(newSize);
    QTest::qWait(50);

    QRect topBarAfter = w->findChild<QFrame*>("topBar")->geometry();
    QVERIFY(topBarAfter.width() > topBarBefore.width());
    QCOMPARE(topBarAfter.height(), topBarBefore.height());
}

/* ----------------------- stress / edge cases -------------------- */
void TestMenuWindow::test_extreme_window_sizes()
{
    QList<QSize> extremes{ QSize(320, 200), QSize(3840, 2160) };
    for (const QSize &s : extremes) {
        w->resize(s);
        QTest::qWait(30);
        QCOMPARE(w->size(), s);
    }
}

void TestMenuWindow::test_multiple_cancel_clicks()
{
    ImageButton *btn1v1 = w->findChildren<ImageButton*>().first();
    QTest::mouseClick(btn1v1, Qt::LeftButton);

    QDialog *dlg = w->findChild<QDialog*>();
    QPushButton *cancel = dlg->findChild<QPushButton*>("", Qt::FindChildrenRecursively);

    for (int i=0;i<10;++i) QTest::mouseClick(cancel, Qt::LeftButton);
    QVERIFY(true); // No debe crashear
}

void TestMenuWindow::test_search_dialog_show_hide_rapidly()
{
    ImageButton *btn1v1 = w->findChildren<ImageButton*>().first();
    for (int i=0;i<4;++i) {
        QTest::mouseClick(btn1v1, Qt::LeftButton);
        QDialog *dlg = w->findChild<QDialog*>();
        QVERIFY(dlg);
        dlg->close();
    }
}

void TestMenuWindow::cleanupTestCase()
{
    closeAllPopups(w);
}


