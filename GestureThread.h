#pragma once

#include <QThread>
#include "util_pipeline.h"

class MyPipeline;
class VirtualTouchScreen;

class GestureThread : public QThread
{
	Q_OBJECT
public:
	explicit GestureThread(VirtualTouchScreen *obj);
	~GestureThread();
protected:
	void run();
signals:
	void moveCursor(int x, int y);
	void tap(int x, int y);
	void showCoords(int x, int y);
private:
	void setupPipeline();
	MyPipeline *pipeline;
	PXCGesture * gesture;
	VirtualTouchScreen *mainWnd;
};
