#ifndef VirtualTouchScreen_H
#define VirtualTouchScreen_H

#include <QtWidgets/QMainWindow>
#include <Windows.h>
#include <QPoint>
#include <QVector>
#include "util_pipeline.h"

class GestureThread;
class GestureAlgos;
class ConfigDialog;

class VirtualTouchScreen : public QMainWindow
{
	Q_OBJECT
	friend class GestureThread;
	friend class ConfigDialog;
public:
	explicit VirtualTouchScreen(QWidget *parent = 0);
	~VirtualTouchScreen();
	enum {OFFSET_X = 200, OFFSET_Y = 100, 
		SCALE_FACTOR_x100 = 100};
	enum Hand {THUMB = 0, INDEX, MIDDLE, RING, PINKY, CENTER, ELBOW, POINTS};
public slots:
	void showMenu();
	void showHelp();
	void onMoveHand(const QPoint pt);
	void onTap(const QPoint &pt);
	void onShowCoords(const QPoint &pt);
	void onSwipe(BYTE code);

protected:
	void paintEvent(QPaintEvent*);

private:
	void setupActions();
	void loadPointer(const QString &path, int size);
	void loadSettings();
	void saveSettings();
	void drawLine(QPainter& p, int p1, int p2);
	GestureThread *gestureThread;
	GestureAlgos *gestureAlgos;
	QString pointerIconPath;
	QPoint offset;
	float scaleFactor;
	ConfigDialog *config;
	QVector<QPoint> handSkeletonPoints_;
};

#endif // VirtualTouchScreen_H
