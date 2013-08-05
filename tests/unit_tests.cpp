#include <QtTest/QtTest>
#include <QObject>

#include "GestureAlgos.h"

class TestGestureAlgos: public QObject
{
    Q_OBJECT
private slots:
    void imageToScreenFilterKalman();
	void filterBiquad();
};

void TestGestureAlgos::imageToScreenFilterKalman()
{
	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);
	float x = 0;
	float y = 0;
	QVERIFY(EXIT_FAILURE == algos->filterKalman(x, y));
	QVERIFY(EXIT_FAILURE == algos->imageToScreen(x, y));
	algos->setScreenSize(1024, 768);
	QVERIFY(EXIT_SUCCESS == algos->filterKalman(x, y));
	QVERIFY(EXIT_FAILURE == algos->imageToScreen(x, y));
	algos->setImageSize(320, 240);
	QVERIFY(EXIT_SUCCESS == algos->filterKalman(x, y));
	QVERIFY(EXIT_FAILURE == algos->imageToScreen(x, y));
	algos->setCorrectionFactors(1.0, 0, 0);
	QVERIFY(EXIT_SUCCESS == algos->filterKalman(x, y));
	x = 0;
	y = 0;
	QVERIFY(EXIT_SUCCESS == algos->imageToScreen(x, y));
	QVERIFY(1024.0 == x);
	QVERIFY(0.0 == y);
}

void TestGestureAlgos::filterBiquad()
{
	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);

	float fn = static_cast<float>(0.1);
	const static float PI = static_cast<float>(3.1415926535);
	const static float TOL = static_cast<float>(1e-3);
	int nbSamp = static_cast<int>(10/fn);
	float val = 0;
	float sum = 0;
	for (int n = 0; n < nbSamp; ++n) {
		val = sin(2*PI*fn*n);
		algos->filterBiquad(val);
		sum += abs(val*val);
	}
	QVERIFY(fabs(48.5314 - sum) < TOL);

	fn = static_cast<float>(0.4);
	nbSamp = static_cast<int>(10/fn);
	sum = 0;
	for (int n = 0; n < nbSamp; ++n) {
		val = sin(2*PI*fn*n);
		algos->filterBiquad(val);
		sum += abs(val*val);
	}
	QVERIFY(fabs(1.58709 - sum) < TOL);
}

QTEST_MAIN(TestGestureAlgos)
#include "unit_tests.moc"