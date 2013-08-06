#include <QtTest/QtTest>
#include <QObject>

#include "GestureAlgos.h"

class TestGestureAlgos: public QObject
{
    Q_OBJECT
private slots:
    void imageToScreenFilterKalman();
	void filterLowPass();
	void filterDiff();
	void isTap();
	void isPressAndHold();
	void isSlide();
private:
	int getData(QVector<int> &x, QVector<int> &y, QVector<float> &depth, 
		const QString &fileName);
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

void TestGestureAlgos::filterLowPass()
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
		algos->filterLowPass(val);
		sum += abs(val*val);
	}
	QVERIFY(fabs(48.5314 - sum) < TOL);

	fn = static_cast<float>(0.4);
	nbSamp = static_cast<int>(10/fn);
	sum = 0;
	for (int n = 0; n < nbSamp; ++n) {
		val = sin(2*PI*fn*n);
		algos->filterLowPass(val);
		sum += abs(val*val);
	}
	QVERIFY(fabs(1.58709 - sum) < TOL);
}

void TestGestureAlgos::filterDiff()
{
	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);

	float s = static_cast<float>(0);
	float diffState = static_cast<float>(0);
	algos->filterDiff(s, diffState);
	QVERIFY(0 == s);
	s = static_cast<float>(1.0);
	algos->filterDiff(s, diffState);
	QVERIFY(1.0 == s);
	algos->filterDiff(s, diffState);
	QVERIFY(0 == s);
}

int TestGestureAlgos::getData(QVector<int> &x, QVector<int> &y, QVector<float> &depth, 
		const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return EXIT_FAILURE;
	}
	QTextStream istream(&file);
	
	while (!istream.atEnd()) {
		QString line = istream.readLine();
		int idx = line.indexOf(") = (");
		if (0 <= idx) {
			QRegExp rx("(\\d+) , (\\d+) , (\\d*\\.\\d+)");
			int pos = rx.indexIn(line, idx);
			if (-1 < pos) {
				x.push_back(rx.cap(1).toInt());
				y.push_back(rx.cap(2).toInt());
				depth.push_back(rx.cap(3).toFloat());
			}
		}
	}
	
	file.close();
	return EXIT_SUCCESS;
}

void TestGestureAlgos::isTap()
{
	QVector<int> x;
	QVector<int> y;
	QVector<float> depth;
	QVERIFY(EXIT_SUCCESS == getData(x, y, depth, "../../tests/ULTRABOOK-BC_vts8_tap.LOG"));

	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);

	//start to process from index 800 in order to detect 3 taps
	int count = 0;
	for (int i = 800; i < x.size(); ++i) {		
		algos->filterLowPass(depth[i]);//apply low pass filter to depth signal
		if (algos->isTap(x[i], y[i], depth[i])) {
			if (850 < i) ++count; //skip transition in the filtered signal
		}
	}
	QVERIFY(3 == count);
}

void TestGestureAlgos::isPressAndHold()
{
	QVector<int> x;
	QVector<int> y;
	QVector<float> depth;
	QVERIFY(EXIT_SUCCESS == getData(x, y, depth, "../../tests/ULTRABOOK-BC_vts9_ph.LOG"));

	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);

	//start to process from index 0 in order to detect 4 presses
	int count = 0;
	for (int i = 0; i < x.size(); ++i) {
		algos->filterLowPass(depth[i]);
		if (algos->isPressAndHold(x[i], y[i], depth[i])) {
			++count;
		}
	}
	QVERIFY(4 == count);
}

void TestGestureAlgos::isSlide()
{
	QVector<int> x;
	QVector<int> y;
	QVector<float> depth;
	QVERIFY(EXIT_SUCCESS == getData(x, y, depth, "../../tests/ULTRABOOK-BC_vts10_slide.LOG"));

	GestureAlgos *algos = GestureAlgos::instance();
	QVERIFY(NULL != algos);

	//start to process from index 0 in order to detect 4 presses
	int count = 0;
	for (int i = 0; i < x.size(); ++i) {
		algos->filterLowPass(depth[i]);
		if (algos->isSlide(x[i], y[i], depth[i])) {
			++count;
		}
	}
	QVERIFY(2 == count);
}

QTEST_MAIN(TestGestureAlgos)
#include "unit_tests.moc"