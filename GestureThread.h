#pragma once

#include <QThread>
#include "util_pipeline.h"

class MyPipeline;
class PresenterHelper;

class GestureThread : public QThread
{
	Q_OBJECT
public:
	explicit GestureThread(PresenterHelper *obj);
	~GestureThread();
protected:
	void run();
signals:
	void moveCursor(int x, int y);
	void tap(int x, int y);
	void showCoords(int x, int y);
private:
	void setupPipeline();
	bool isTap(double in);
	enum {MAX_OBS_DURATION = 15, MAX_NB_SAMPLES = 2};
	MyPipeline *pipeline;
	PXCGesture * gesture;
	PresenterHelper *mainWnd;
};
