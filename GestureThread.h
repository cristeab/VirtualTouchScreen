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
	void moveIndex();
	void moveThumb();
	//touch signals
	void touchDown(const QPoint &ptThumb, const QPoint &ptIndex);
	void touchDown(const QPoint &ptIndex);
	void touchUp(const QPoint &ptThumb, const QPoint &ptIndex);
	void touchUp(const QPoint &ptIndex);
private:
	void setupPipeline();
	MyPipeline *pipeline;
	PXCGesture * gesture;
	VirtualTouchScreen *mainWnd;
};
