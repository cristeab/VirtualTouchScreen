#include <qglobal.h>
#include <QDebug>
#include "GestureAlgos.h"

GestureAlgos *GestureAlgos::instance_ = NULL;

GestureAlgos::GestureAlgos() : scrWidth_(0), scrHeight_(0),
	imgWidth_(0), imgHeight_(0),
	scaleFactor_(0.0), offsetX_(0), offsetY_(0),
	KF_(cv::KalmanFilter(4, 2, 0)),
	measurement_(cv::Mat_<float>(2,1))
{
}

int GestureAlgos::initKalman()
{
	if ((0 >= scrWidth_) || (0 >= scrHeight_)) {
		return EXIT_FAILURE;
	}
	//setup Kalman filter
	measurement_.setTo(cv::Scalar(0));
	KF_.statePre.at<float>(0) = static_cast<float>(scrWidth_)/2.0;
	KF_.statePre.at<float>(1) = static_cast<float>(scrHeight_)/2.0;
	KF_.statePre.at<float>(2) = 0;
	KF_.statePre.at<float>(3) = 0;
	KF_.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1,0,0,0,   0,1,0,0,  0,0,1,0,  0,0,0,1);
	setIdentity(KF_.measurementMatrix);
	setIdentity(KF_.processNoiseCov, cv::Scalar::all(1e-4));
	setIdentity(KF_.measurementNoiseCov, cv::Scalar::all(1e-1));
	setIdentity(KF_.errorCovPost, cv::Scalar::all(.1));
	return EXIT_SUCCESS;
}

int GestureAlgos::imageToScreen(float &x, float &y)
{
	int rc = EXIT_SUCCESS;
	//convert from image coodinates to screen coordinates
	if ((0 < imgWidth_) && (0 < imgHeight_))
	{
		x = (x*scrWidth_)/imgWidth_;
		y = (y*scrHeight_)/imgHeight_;
	} else {
		qDebug() << "either image width or image height are not initialized";
		rc = EXIT_FAILURE;
	}
	//invert X axis
	x = scrWidth_-x;
	//apply corrections as needed
	if ((0 < scaleFactor_) && (0 < offsetX_) && (0 < offsetY_)) {
		x = scaleFactor_*(x-offsetX_);
		y = scaleFactor_*(y-offsetY_);
	} else {
		qDebug() << "correction factors have not been initialized";
		rc = EXIT_FAILURE;
	}
	//threshold both coordinates
	if (0 > x) x = 0.0;
	else if (scrWidth_ < x) x = scrWidth_-10;
	if (0 > y) y = 0;
	else if (scrHeight_ < y) y = scrHeight_-10;

	return rc;
}

int GestureAlgos::filterKalman(float &x, float &y)
{
	static bool initDone = false;
	if (!initDone) {
		if (EXIT_FAILURE == initKalman()) {
			return EXIT_FAILURE;
		}
		initDone = true;
	}

	int rc = imageToScreen(x, y);

	KF_.predict();
	measurement_(0) = x;
	measurement_(1) = y;
	cv::Mat estimated = KF_.correct(measurement_);
	x = static_cast<int>(estimated.at<float>(0));
	y = static_cast<int>(estimated.at<float>(1));

	return rc;
}

bool GestureAlgos::isTap(double samp)
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

bool GestureAlgos::isPressAndHold(int x, int y, double depth)
{
	return false;
}
