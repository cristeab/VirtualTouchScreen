#include <QtTest/QtTest>
#include <QObject>

#include "GestureAlgos.h"

class TestGestureAlgos: public QObject
{
    Q_OBJECT
private slots:
    void imageToScreen();
};

void TestGestureAlgos::imageToScreen()
{
	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);
	float x = 0;
	float y = 0;
	QVERIFY(EXIT_FAILURE == algos->filterKalman(x, y));
	QVERIFY(EXIT_FAILURE == algos->imageToScreen(x, y));
	algos->setScreenSize(1024, 768);
	QVERIFY(EXIT_FAILURE == algos->filterKalman(x, y));
	QVERIFY(EXIT_FAILURE == algos->imageToScreen(x, y));
	algos->setImageSize(320, 240);
	QVERIFY(EXIT_FAILURE == algos->filterKalman(x, y));
	QVERIFY(EXIT_FAILURE == algos->imageToScreen(x, y));
	algos->setCorrectionFactors(1.0, 0, 0);
	QVERIFY(EXIT_SUCCESS == algos->filterKalman(x, y));
	QVERIFY(EXIT_SUCCESS == algos->imageToScreen(x, y));
}

QTEST_MAIN(TestGestureAlgos)
#include "unit_tests.moc"