#include <QMessageBox>
#include <QDebug>
#include "PresenterHelper.h"
#include "GestureThread.h"

class MyPipeline : public UtilPipeline
{
public:
	explicit MyPipeline(PresenterHelper *p) : UtilPipeline(), pres(p)
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
private:
	PresenterHelper *pres;
};

GestureThread::GestureThread(PresenterHelper *obj) : QThread(), mainWnd(obj)
{
	setupPipeline();
	connect(this, SIGNAL(moveCursor(int,int)), mainWnd, SLOT(onMoveCursor(int,int)));
	connect(this, SIGNAL(tap(int,int)), mainWnd, SLOT(onTap(int,int)));
	connect(this, SIGNAL(showCoords(int,int)), mainWnd, SLOT(onShowCoords(int,int)));
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

			PXCGesture::GeoNode handNode;
			if(gesture->QueryNodeData(0, PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,
				&handNode) != PXC_STATUS_ITEM_UNAVAILABLE)
			{
				double imgX = static_cast<double>(handNode.positionImage.x);
				double imgY = static_cast<double>(handNode.positionImage.y);
				double depth = static_cast<double>(handNode.positionWorld.y);

				if ((0 < mainWnd->imgWidth) && (0 < mainWnd->imgHeight))
				{
					imgX = (imgX*mainWnd->scrWidth)/mainWnd->imgWidth;
					imgY = (imgY*mainWnd->scrHeight)/mainWnd->imgHeight;
				} else
				{
					qDebug() << "either image width or image height are not initialized";
				}
				//invert X axis
				imgX = mainWnd->scrWidth-imgX;
				//apply corrections as needed
				imgX = mainWnd->scaleFactor*(imgX-mainWnd->offsetX);
				imgY = mainWnd->scaleFactor*(imgY-mainWnd->offsetY);
				//threshold both coordinates
				if (0 > imgX) imgX = 0.0;
				else if (mainWnd->scrWidth < imgX) imgX = mainWnd->scrWidth-10;
				if (0 > imgY) imgY = 0;
				else if (mainWnd->scrHeight < imgY) imgY = mainWnd->scrHeight-10;
				//move cursor to the new position
				qDebug() << QThread::currentThreadId() << "(x,y,d) = (" << imgX << "," << imgY << "," << depth << ")";
				int x = static_cast<int>(imgX);
				int y = static_cast<int>(imgY);
				emit moveCursor(x, y);
				//check for tap
				if(isTap(depth))
				{
					qDebug() << "tap detected";
					emit tap(x, y);
				}
				//display coordinates if requested
				if (mainWnd->showCoords)
				{
					emit showCoords(x, y);
				}
			}

			// we must release the frame
			pipeline->ReleaseFrame();
		}
	}
}

bool GestureThread::isTap(double samp)
{
	static double prevSamp = 0.0;
	static int obsDuration = 0;
	static int varSign = 0;
	static int nbSignSamp = 0;

	bool out = false;
	double diffSamp = samp-prevSamp;

	if (qAbs(diffSamp) > 0.05)
	{
		int sign = static_cast<int>((diffSamp)/qAbs(diffSamp));
		if (0 == varSign)
		{
			varSign = sign;
			obsDuration = 0;
			nbSignSamp = 0;
		}
		else if (varSign == sign)
		{
			obsDuration = 0;
			++nbSignSamp;
		}
		else
		{
			if (MAX_NB_SAMPLES < nbSignSamp)
			{
				out = true;
				obsDuration = 0;
				varSign = 0;
			}
			nbSignSamp = 0;
		}
	}
	prevSamp = samp;

	if (0 != varSign)
	{
		++obsDuration;
	}
	if (MAX_OBS_DURATION < obsDuration)
	{
		obsDuration = 0;
		varSign = 0;
		nbSignSamp = 0;
	}

	return out;
}

void GestureThread::setupPipeline()
{
	pipeline = new MyPipeline(mainWnd);
	pipeline->EnableGesture();
	if (pipeline->Init())
	{
		if (!pipeline->QueryImageSize(PXCImage::IMAGE_TYPE_DEPTH, mainWnd->imgWidth, mainWnd->imgHeight))
		{
			mainWnd->imgWidth = -1;
			mainWnd->imgHeight = -1;
		}
	} else
	{
		QMessageBox::warning(NULL, "Presenter Helper", "Cannot init gesture camera");
		::TerminateProcess(::GetCurrentProcess(), EXIT_FAILURE);
	}
}
