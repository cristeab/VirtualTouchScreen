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
	connect(this, SIGNAL(moveHand(const QPoint)), mainWnd, SLOT(onMoveHand(const QPoint)));
	connect(this, SIGNAL(tap(const QPoint&)), mainWnd, SLOT(onTap(const QPoint&)));
	connect(this, SIGNAL(showCoords(const QPoint&)), mainWnd, SLOT(onShowCoords(const QPoint&)));
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
			QPoint refHandPos;
			float depth = 0;
			if(gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,
				&handNode) != PXC_STATUS_ITEM_UNAVAILABLE)
			{
				qDebug() << "center: x = " << handNode.positionImage.x << 
						", y = " << handNode.positionImage.y;

				refHandPos.setX(handNode.positionImage.x);
				refHandPos.setY(handNode.positionImage.y);
				depth = static_cast<float>(handNode.positionWorld.y);

				//convert to screen coordinates
				QPoint handPos = refHandPos;//TODO: remove this
				mainWnd->gestureAlgos->imageToScreen(handPos);

				//filter hand coordinates (center only for now)
				if (EXIT_FAILURE == mainWnd->gestureAlgos->filterKalman(handPos)) {
					qDebug() << "error in Kalman filter";
				}

				//filter depth information: low pass filtering in order to remove high frequency components
				mainWnd->gestureAlgos->filterLowPass(depth);

				qDebug() << QThread::currentThreadId() << "(x,y,d) = (" << handPos.x()
					<< "," << handPos.y() << "," << depth << ")";

				//move cursor to the new position
				emit moveHand(handPos);//TODO: no window movement
				//check hand status
				if (handNode.opennessState & PXCGesture::GeoNode::Openness::LABEL_OPEN) {
					qDebug() << "hand open";
				} else if (handNode.opennessState & PXCGesture::GeoNode::Openness::LABEL_CLOSE) {
					qDebug() << "hand closed";
				}
			} else {
				updateHand = false;
			}

			//finger position
			PXCGesture::GeoNode fingerNode[5];
			if ( gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY |
				PXCGesture::GeoNode::LABEL_FINGER_THUMB, 
				5, fingerNode) != PXC_STATUS_ITEM_UNAVAILABLE) {
					for (int i = 0; i < 5; ++i) {
						switch (fingerNode[i].body & PXCGesture::GeoNode::LABEL_MASK_DETAILS) {
						case PXCGesture::GeoNode::LABEL_FINGER_THUMB:
							qDebug() << "thumb: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y;
							break;
						case PXCGesture::GeoNode::LABEL_FINGER_INDEX:
							qDebug() << "index: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y;
							break;
						case PXCGesture::GeoNode::LABEL_FINGER_MIDDLE:
							qDebug() << "middle: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y;
							break;
						case PXCGesture::GeoNode::LABEL_FINGER_RING:
							qDebug() << "ring: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y;
							break;
						case PXCGesture::GeoNode::LABEL_FINGER_PINKY:
							qDebug() << "pinky: x = " << fingerNode[i].positionImage.x << 
								", y = " << fingerNode[i].positionImage.y;
							break;
						default:
							updateHand = false;
						}
					}
			} else {
				updateHand = false;
			}

			//elbow position
			PXCGesture::GeoNode elbowNode;
			if(gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_ELBOW_PRIMARY,
				&elbowNode) != PXC_STATUS_ITEM_UNAVAILABLE) {
					qDebug() << "elbow: x = " << elbowNode.positionImage.x << 
						", y = " << elbowNode.positionImage.y;
			} else {
				updateHand = false;
			}

			if (updateHand) {
				//fingers
				int i = 0;
				mainWnd->skeletonPointMutex_.lock();
				for (; i < 5; ++i) {
					mainWnd->handSkeletonPoints_[i] = QPoint(fingerNode[i].positionImage.x, fingerNode[i].positionImage.y);
					mainWnd->gestureAlgos->toHandCenter(mainWnd->handSkeletonPoints_[i], refHandPos);
				}
				//hand center
				mainWnd->handSkeletonPoints_[i++] = mainWnd->gestureAlgos->imageCenter();
				//elbow
				mainWnd->handSkeletonPoints_[i] = QPoint(elbowNode.positionImage.x, elbowNode.positionImage.y);
				mainWnd->gestureAlgos->toHandCenter(mainWnd->handSkeletonPoints_[i], refHandPos);
				mainWnd->skeletonPointMutex_.unlock();
				//request hand skeleton redraw
				emit updateHandSkeleton();
				//detect gesture
				switch (mainWnd->gestureAlgos->isTouch(mainWnd->handSkeletonPoints_[0],
					mainWnd->handSkeletonPoints_[1], depth))
				{
				case GestureAlgos::TouchType::DOUBLE_DOWN:
					qDebug() << "double touch down";
					emit touchDown(mainWnd->handSkeletonPoints_[0], mainWnd->handSkeletonPoints_[1]);
					break;
				case GestureAlgos::TouchType::SINGLE_DOWN:
					qDebug() << "single touch down";
					emit touchDown(mainWnd->handSkeletonPoints_[1]);
					break;
				case GestureAlgos::TouchType::DOUBLE_UP:
					qDebug() << "double touch up";
					emit touchUp(mainWnd->handSkeletonPoints_[0], mainWnd->handSkeletonPoints_[1]);
					//emit tap(handPos);
					break;
				case GestureAlgos::TouchType::SINGLE_UP:
					qDebug() << "single touch up";
					emit touchUp(mainWnd->handSkeletonPoints_[1]);
					break;
				default:
					(void)0;
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
