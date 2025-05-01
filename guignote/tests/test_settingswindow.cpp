#include "test_settingswindow.h"

#include "settingswindow.h"
#include <QSlider>
#include <QPushButton>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QAudioOutput>
#include <QFile>

using namespace Qt::StringLiterals;

// ----- DummyMenuWindow ------------------------------------------------------
class DummyMenuWindow : public QWidget
{
    Q_OBJECT
public:
    // expos PUBLIC la señal que SettingsWindow conecta
    void setVolume(int v) { volumeReceived = v; }
    int volumeReceived = -1;

    // campo opcional para que SettingsWindow no falle al leerlo
    QAudioOutput *audioOutput = nullptr;
};
// ---------------------------------------------------------------------------


/* ====================================================================== */
/*  FIXTURE                                                               */
/* ====================================================================== */
static const QString kUser    = QStringLiteral("unittest_usr");
static const QString kOrgName = QStringLiteral("Grace Hopper");
static QString confPath() {
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
    +  "/"+kOrgName+"/Sota, Caballo y Rey_" + kUser + ".conf";
}

void TestSettingsWindow::init()
{
    /*  Limpiamos cualquier conf que haya quedado de tests anteriores   */
    QFile::remove(confPath());

    /*  Creamos MenuWindow ficticia + SettingsWindow bajo prueba        */
    auto *dummy = new DummyMenuWindow;
    w = new SettingsWindow(dummy, nullptr, kUser);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));
}

void TestSettingsWindow::cleanup()
{
    delete w;  w = nullptr;
    QFile::remove(confPath());
}

/* ====================================================================== */
/*  CASOS DE PRUEBA                                                       */
/* ====================================================================== */
void TestSettingsWindow::test_basic_widgets_exist()
{
    QCOMPARE(w->findChildren<QSlider*>().size(), 2);              // música + efectos
    QVERIFY(w->findChild<QPushButton*>());                       // hay al menos un botón (la ✖)
}

void TestSettingsWindow::test_close_button_closes()
{
    QPushButton *close = w->findChild<QPushButton*>();
    QVERIFY(close);
    QTest::mouseClick(close, Qt::LeftButton);
    QTRY_VERIFY(!w->isVisible());                                // cerró en <1 s
}

void TestSettingsWindow::test_slider_emits_volume()
{
    // 1) Re-creamos con un dummy al que SÍ vamos a "espiar" el volumen
    delete w;
    auto *dummy = new DummyMenuWindow;
    w = new SettingsWindow(dummy, /*parent=*/nullptr, kUser);
    w->show();
    QVERIFY(QTest::qWaitForWindowExposed(w));

    // 2) Obtenemos el QSlider de música
    QSlider *music = w->findChild<QSlider*>();
    QVERIFY(music);

    // 3) Conectamos el signal valueChanged del slider al slot de nuestro dummy
    connect(music, &QSlider::valueChanged,
            dummy,  &DummyMenuWindow::setVolume);

    // 4) Disparamos el cambio de valor
    music->setValue(33);
    QTest::qWait(10);  // damos tiempo a procesar el evento

    // 5) Verificamos que el dummy recibió el valor
    QCOMPARE(dummy->volumeReceived, 33);

    delete dummy;
}


void TestSettingsWindow::test_save_and_load_settings()
{
    /*  1) cambiamos valores y cerramos => debe persistir              */
    QSlider *music   = w->findChildren<QSlider*>().at(0);
    QSlider *effects = w->findChildren<QSlider*>().at(1);
    music  ->setValue(77);
    effects->setValue(12);
    w->close();                                // llama saveSettings()

    /*  2) abrimos otra ventana: loadSettings() debe restaurar         */
    delete w;
    w = new SettingsWindow(new DummyMenuWindow, nullptr, kUser);
    QCOMPARE(w->findChildren<QSlider*>().at(0)->value(), 77);
    QCOMPARE(w->findChildren<QSlider*>().at(1)->value(), 12);
}

#include "test_settingswindow.moc"
