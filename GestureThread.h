#pragma once

#include <QThread>
#include <QPoint>
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
	void moveHand(const QPoint pt);
	void tap(const QPoint &pt);
	void showCoords(const QPoint &pt);
	void updateHandSkeleton();
private:
	void setupPipeline();
	MyPipeline *pipeline;
	PXCGesture * gesture;
	VirtualTouchScreen *mainWnd;
};
