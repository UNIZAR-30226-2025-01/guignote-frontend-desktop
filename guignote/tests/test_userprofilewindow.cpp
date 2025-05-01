#include "test_userprofilewindow.h"
#include <QPushButton>
#include <QLabel>
#include "icon.h"
using namespace Qt::StringLiterals;


/* -------------- fixture ----------------- */
void TestUserProfileWindow::init()
{
    w = new UserProfileWindow();   // sin parent
    w->show();
    QVERIFY( QTest::qWaitForWindowExposed(w) );
}

void TestUserProfileWindow::cleanup()
{
    delete w;
    w = nullptr;
}

/* -------------- casos ------------------- */
void TestUserProfileWindow::test_basic_widgets_exist()
{
    /* Icon de la foto */
    QVERIFY( w->findChild<Icon*>() );

    /* botón de cierre */
    QPushButton *closeBtn = w->findChild<QPushButton*>(QString(),
                                                        Qt::FindDirectChildrenOnly);
    QVERIFY(closeBtn);

    /* dos botones de la parte inferior (por texto) */
    QList<QPushButton*> bottoms =
        w->findChildren<QPushButton*>(QString(),
                                       Qt::FindChildrenRecursively);
    int friendly = 0, team = 0;
    for (auto *b : bottoms) {
        if (b->text() == u"Partida Amistosa"_s) ++friendly;
        if (b->text() == u"Jugar en Equipo"_s) ++team;
    }
    QCOMPARE(friendly,1);
    QCOMPARE(team,1);
}

void TestUserProfileWindow::test_window_flags_and_size()
{
    /* Frameless + Dialog */
    QVERIFY( w->windowFlags() & Qt::FramelessWindowHint );
    QVERIFY( w->windowFlags() & Qt::Dialog );

    /* Tamaño fijo (mínimo = máximo) */
    QCOMPARE( w->minimumSize(), w->maximumSize() );
}

void TestUserProfileWindow::test_close_button_closes()
{
    QPushButton *closeBtn =
        w->findChild<QPushButton*>(QString(), Qt::FindDirectChildrenOnly);
    QVERIFY(closeBtn);

    QTest::mouseClick(closeBtn, Qt::LeftButton);
    QTRY_VERIFY( !w->isVisible() );
}

void TestUserProfileWindow::test_circular_pixmap_shape()
{
    Icon *foto = w->findChild<Icon*>();
    QVERIFY(foto);
    QPixmap px = foto->pixmap(Qt::ReturnByValue);

    QVERIFY(!px.isNull());
    QImage img = px.toImage();

    int sz = img.width();                  // es cuadrado (200×200)
    /* centro opaco */
    QVERIFY( img.pixelColor(sz/2, sz/2).alpha() > 200 );

    /* esquinas transparentes */
    QVector<QPoint> corners = { {0,0}, {0,sz-1}, {sz-1,0}, {sz-1,sz-1} };
    for (auto p : corners)
        QCOMPARE( img.pixelColor(p).alpha(), 0 );
}

void TestUserProfileWindow::test_bottom_buttons_clickable()
{
    auto buttons = w->findChildren<QPushButton*>();
    for (QPushButton *b : buttons)
        if (b->text() == u"Partida Amistosa"_s ||
            b->text() == u"Jugar en Equipo"_s)
        {
            QVERIFY(b->isEnabled());
            QTest::mouseClick(b, Qt::LeftButton);   // no debería lanzar excepción
        }
}
