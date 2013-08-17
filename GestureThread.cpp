#include <QMessageBox>
#include <QDebug>
#include "VirtualTouchScreen.h"
#include "GestureThread.h"
#include "GestureAlgos.h"

class MyPipeline : public UtilPipeline
{
public:
	explicit MyPipeline(VirtualTouchScreen *p) : UtilPipeline(), pres(p)
	{
		//for some reason only default values are accepted
		//EnableImage(PXCImage::IMAGE_TYPE_DEPTH, 320, 240);
	}
	void PXCAPI OnGesture(PXCGesture::Gesture *data)
	{
		if (data->label & PXCGesture::Gesture::LABEL_NAV_SWIPE_LEFT) {
			qDebug() << "Gesture detected: swipe left";
			if (NULL != pres)
			{
				pres->onSwipe(VK_RIGHT);//more natural swipe
			}
		} else if (data->label & PXCGesture::Gesture::LABEL_NAV_SWIPE_RIGHT) {
			qDebug() << "Gesture detected: swipe right";
			if (NULL != pres)
			{
				pres->onSwipe(VK_LEFT);
			}
		} else if (data->label & PXCGesture::Gesture::LABEL_HAND_CIRCLE) {
			qDebug() << "Gesture detected: circle";
		}
	}
	void PXCAPI OnAlert(PXCGesture::Alert *alert)
	{
		if (alert->label & PXCGesture::Alert::LABEL_FOV_BOTTOM) {
			qDebug() << "The tracking object touches the bottom field of view";
		} else if (alert->label & PXCGesture::Alert::LABEL_FOV_LEFT) {
			qDebug() << "The tracking object touches the left field of view";
		} else if (alert->label & PXCGesture::Alert::LABEL_FOV_RIGHT) {
			qDebug() << "The tracking object touches the right field of view";
		} else if (alert->label & PXCGesture::Alert::LABEL_FOV_TOP) {
			qDebug() << "The tracking object touches the top field of view";
		} else if (alert->label & PXCGesture::Alert::LABEL_FOV_OK) {
			qDebug() << "The tracking object is within field of view";
		} else if (alert->label & PXCGesture::Alert::LABEL_FOV_BLOCKED) {
			qDebug() << "The field of view is blocked";
		} else if (alert->label & PXCGesture::Alert::LABEL_GEONODE_ACTIVE) {
			qDebug() << "The tracking object is found";
		} else if (alert->label & PXCGesture::Alert::LABEL_GEONODE_INACTIVE) {
			qDebug() << "The tracking object is lost";
		}
	}
private:
	VirtualTouchScreen *pres;
};

GestureThread::GestureThread(VirtualTouchScreen *obj) : QThread(), 
	mainWnd(obj)
{
	setupPipeline();
	connect(this, SIGNAL(moveHand()), mainWnd, SLOT(onMoveHand()));
	connect(this, SIGNAL(showThumb(bool)), mainWnd, SLOT(onShowThumb(bool)));
	//link touch signals with the corresponding slots in the main window
	connect(this, SIGNAL(touchDown(const QPoint&, const QPoint&)), mainWnd, 
		SLOT(onTouchDown(const QPoint&, const QPoint&)));
	connect(this, SIGNAL(touchDown(const QPoint&)), mainWnd, 
		SLOT(onTouchDown(const QPoint&)));
	connect(this, SIGNAL(touchUp(const QPoint&, const QPoint&)), mainWnd, 
		SLOT(onTouchUp(const QPoint&, const QPoint&)));
	connect(this, SIGNAL(touchUp(const QPoint&)), mainWnd, 
		SLOT(onTouchUp(const QPoint&)));
}

GestureThread::~GestureThread()
{
	delete pipeline;
}

void GestureThread::run()
{
	while (true)
	{
		if(pipeline->AcquireFrame(true))
		{
			gesture = pipeline->QueryGesture();
			bool updateHand = true;

			//hand position
			PXCGesture::GeoNode handNode;
			if(gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,
				&handNode) != PXC_STATUS_ITEM_UNAVAILABLE)
			{
				//check hand status
				if (handNode.opennessState & PXCGesture::GeoNode::Openness::LABEL_OPEN) {					
					qDebug() << "hand open";
					emit showThumb(true);
				} else if (handNode.opennessState & PXCGesture::GeoNode::Openness::LABEL_CLOSE) {
					qDebug() << "hand closed";
					emit showThumb(false);
				}
			}

			//finger position
			PXCGesture::GeoNode fingerNode[VirtualTouchScreen::POINTS];
			int i = 0;
			if ( gesture->QueryNodeData(0, 
				PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY | PXCGesture::GeoNode::LABEL_FINGER_THUMB, 
				VirtualTouchScreen::POINTS, fingerNode) != PXC_STATUS_ITEM_UNAVAILABLE) {
					for (i = 0; i < VirtualTouchScreen::POINTS; ++i) {
						switch (fingerNode[i].body & PXCGesture::GeoNode::LABEL_MASK_DETAILS) {
						case PXCGesture::GeoNode::LABEL_FINGER_THUMB:
							qDebug() << "thumb: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y <<
								", depth = " << fingerNode[i].positionWorld.y;
							break;
						case PXCGesture::GeoNode::LABEL_FINGER_INDEX:
							qDebug() << "index: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y <<
								", depth = " << fingerNode[i].positionWorld.y;
							break;
						default:
							updateHand = false;
						}
					}
			} else {
				updateHand = false;
			}

			if (updateHand) {
				//fingers
				mainWnd->skeletonPointMutex_.lock();
				qreal depth[VirtualTouchScreen::POINTS];
				for (i = 0; i < VirtualTouchScreen::POINTS; ++i) {
					mainWnd->handSkeletonPoints_[i] = QPointF(fingerNode[i].positionImage.x, fingerNode[i].positionImage.y);
					mainWnd->gestureAlgos->imageToScreen(mainWnd->handSkeletonPoints_[i]);
					depth[i] = static_cast<qreal>(fingerNode[i].positionWorld.y);					
				}
				//Kalman filter for coordinates
				mainWnd->gestureAlgos->filterKalman(
					mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB],
					mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX]);
				mainWnd->skeletonPointMutex_.unlock();
				//low pass filter for the depth
				mainWnd->gestureAlgos->filterLowPass(depth[0], depth[1]);

				//hand position update
				emit moveHand();

				//detect touch gesture				
				switch (mainWnd->gestureAlgos->isTouch(depth[VirtualTouchScreen::THUMB], 
					depth[VirtualTouchScreen::INDEX]))
				{
				case GestureAlgos::TouchType::DOUBLE_DOWN:
					qDebug() << "sending double touch down";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint(), 
						mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					break;
				case GestureAlgos::TouchType::INDEX_DOWN:
					qDebug() << "sending index touch down";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					break;
				case GestureAlgos::TouchType::THUMB_DOWN:
					qDebug() << "sending thumb touch down";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint());
					break;
				case GestureAlgos::TouchType::DOUBLE_UP:
					qDebug() << "sending double touch up";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint(), 
						mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());//needed by touch up
					emit touchUp(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint(), 
						mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					break;
				case GestureAlgos::TouchType::INDEX_UP:
					qDebug() << "sending index touch up";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					emit touchUp(mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					break;
				case GestureAlgos::TouchType::THUMB_UP:
					qDebug() << "sending thumb touch up";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint());
					emit touchUp(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint());
					break;
				case GestureAlgos::TouchType::INDEX_DOWN_THUMB_UP:
					qDebug() << "sending index down thumb up";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint());
					emit touchUp(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint());
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					break;
				case GestureAlgos::TouchType::INDEX_UP_THUMB_DOWN:
					qDebug() << "sending index up thumb down";
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					emit touchUp(mainWnd->handSkeletonPoints_[VirtualTouchScreen::INDEX].toPoint());
					emit touchDown(mainWnd->handSkeletonPoints_[VirtualTouchScreen::THUMB].toPoint());					
					break;
				case GestureAlgos::TouchType::NONE:
					qDebug() << "touch type is none";
					break;
				default:
					qDebug() << "Error: unknown touch type";
				}
			}

			// we must release the frame
			pipeline->ReleaseFrame();
		}
	}
}

void GestureThread::setupPipeline()
{
	pipeline = new MyPipeline(mainWnd);
	pipeline->EnableGesture();
	if (pipeline->Init())
	{
		pxcU32 imgWidth;
		pxcU32 imgHeight;
		if (!pipeline->QueryImageSize(PXCImage::IMAGE_TYPE_DEPTH, imgWidth, imgHeight))
		{
			imgWidth = imgHeight = -1;
		}
		mainWnd->gestureAlgos->setImageSize(QSize(imgWidth, imgHeight));
	} else
	{
		QMessageBox::warning(NULL, "Presenter Helper", "Cannot init gesture camera");
		::TerminateProcess(::GetCurrentProcess(), EXIT_FAILURE);
	}
}
