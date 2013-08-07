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
		switch (data->label)
		{
		case PXCGesture::Gesture::LABEL_NAV_SWIPE_LEFT:
			qDebug() << "Gesture detected: swipe left";
			if (NULL != pres)
			{
				pres->onSwipe(VK_RIGHT);//more natural swipe
			}
			break;
		case PXCGesture::Gesture::LABEL_NAV_SWIPE_RIGHT:
			qDebug() << "Gesture detected: swipe right";
			if (NULL != pres)
			{
				pres->onSwipe(VK_LEFT);
			}
			break;
		default:
			(void)0;
		}
	}
	void PXCAPI OnAlert(PXCGesture::Alert *alert)
	{
		switch (alert->label)
		{
		case PXCGesture::Alert::LABEL_FOV_BOTTOM:
			qDebug() << "The tracking object touches the bottom field of view";
			break;
		case PXCGesture::Alert::LABEL_FOV_LEFT:
			qDebug() << "The tracking object touches the left field of view";
			break;
		case PXCGesture::Alert::LABEL_FOV_RIGHT:
			qDebug() << "The tracking object touches the right field of view";
			break;
		case PXCGesture::Alert::LABEL_FOV_TOP:
			qDebug() << "The tracking object touches the top field of view";
			break;
		case PXCGesture::Alert::LABEL_FOV_OK:
			qDebug() << "The tracking object is within field of view";
			break;
		case PXCGesture::Alert::LABEL_FOV_BLOCKED:
			qDebug() << "The field of view is blocked";
			break;
		case PXCGesture::Alert::LABEL_GEONODE_ACTIVE:
			qDebug() << "The tracking object is found";
			break;
		case PXCGesture::Alert::LABEL_GEONODE_INACTIVE:
			qDebug() << "The tracking object is lost";
			break;
		default:
			(void)0;
		}
	}
private:
	VirtualTouchScreen *pres;
};

GestureThread::GestureThread(VirtualTouchScreen *obj) : QThread(), mainWnd(obj)
{
	setupPipeline();
	connect(this, SIGNAL(moveHand(const QPoint&)), mainWnd, SLOT(onMoveHand(const QPoint&)));
	connect(this, SIGNAL(tap(const QPoint&)), mainWnd, SLOT(onTap(const QPoint&)));
	connect(this, SIGNAL(showCoords(const QPoint&)), mainWnd, SLOT(onShowCoords(const QPoint&)));
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

			//hand position
			PXCGesture::GeoNode handNode;
			if(gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,
				&handNode) != PXC_STATUS_ITEM_UNAVAILABLE)
			{
				QPoint handPos(handNode.positionImage.x, handNode.positionImage.y);
				float depth = static_cast<float>(handNode.positionWorld.y);

				//convert to screen coordinates
				mainWnd->gestureAlgos->imageToScreen(handPos);

				//filter data
				if (EXIT_FAILURE == mainWnd->gestureAlgos->filterKalman(handPos)) {
					qDebug() << "error in Kalman filter";
				}
				mainWnd->gestureAlgos->filterLowPass(depth);

				qDebug() << QThread::currentThreadId() << "(x,y,d) = (" << handPos.x()
					<< "," << handPos.y() << "," << depth << ")";

				//move cursor to the new position
				emit moveHand(handPos);//TODO: no window movement
				//check for tap
				if(mainWnd->gestureAlgos->isTap(handPos, depth))
				{
					qDebug() << "tap detected";
					emit tap(handPos);
				}
				//display coordinates if requested
				/*if (mainWnd->showCoords)
				{
					emit showCoords(x, y);
				}*/
				//check hand status
				switch (handNode.opennessState)
				{
				case PXCGesture::GeoNode::Openness::LABEL_OPEN:
					qDebug() << "hand open";
					break;
				case PXCGesture::GeoNode::Openness::LABEL_CLOSE:
					qDebug() << "hand closed";
					break;
				default:
					(void)0;
				}
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
							(void)0;
						}
					}
			}

			//elbow position
			PXCGesture::GeoNode elbowNode;
			if(gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_ELBOW_PRIMARY,
				&elbowNode) != PXC_STATUS_ITEM_UNAVAILABLE) {
					qDebug() << "elbow: x = " << elbowNode.positionImage.x << 
						", y = " << elbowNode.positionImage.y;
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
