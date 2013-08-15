#include <qglobal.h>
#include <QDebug>
#include "GestureAlgos.h"

GestureAlgos::GestureAlgos() :
	screen_(0, 0),
	image_(0,0),
	scaleFactor_(0.0), offset_(0,0),
	KF_(cv::KalmanFilter(8, 4, 0)),
	measurement_(cv::Mat_<float>(4,1))
{
}

int GestureAlgos::initKalman()
{
	if ((0 >= screen_.width()) || (0 >= screen_.height())) {
		return EXIT_FAILURE;
	}
	//setup Kalman filter
	measurement_.setTo(cv::Scalar(0));
	KF_.statePre.at<float>(0) = static_cast<float>(screen_.width())/2.0;//x0
	KF_.statePre.at<float>(1) = static_cast<float>(screen_.height())/2.0;//y0
	KF_.statePre.at<float>(2) = 0;//dx0
	KF_.statePre.at<float>(3) = 0;//dy0
	KF_.statePre.at<float>(4) = static_cast<float>(screen_.width())/2.0;//x1
	KF_.statePre.at<float>(5) = static_cast<float>(screen_.height())/2.0;//y1
	KF_.statePre.at<float>(6) = 0;//dx1
	KF_.statePre.at<float>(7) = 0;//dy1
	KF_.transitionMatrix = *(cv::Mat_<float>(8, 8) << 
	//  x0,y0,dx0,dy0,x1,y2,dx1,dy1,
		1,0,0,0,0,0,0,0,   
		0,1,0,0,0,0,0,0,  
		0,0,1,0,0,0,0,0,  
		0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,0,0,
		0,0,0,0,0,1,0,0,
		0,0,0,0,0,0,1,0,
		0,0,0,0,0,0,0,1);
	setIdentity(KF_.measurementMatrix);
	setIdentity(KF_.processNoiseCov, cv::Scalar::all(1e-4));
	setIdentity(KF_.measurementNoiseCov, cv::Scalar::all(1e-1));
	setIdentity(KF_.errorCovPost, cv::Scalar::all(.1));
	return EXIT_SUCCESS;
}

int GestureAlgos::imageToScreen(QPointF &pt)
{
	int rc = EXIT_SUCCESS;

	//convert to double in order to have the highest precision
	qreal x = pt.x();
	qreal y = pt.y();

	//convert from image coodinates to screen coordinates
	if ((0 < image_.width()) && (0 < image_.height()))
	{
		x = (x*screen_.width())/image_.width();
		y = (y*screen_.height())/image_.height();
	} else {
		qDebug() << "either image width or image height is not initialized";
		rc = EXIT_FAILURE;
	}
	//invert X axis
	x = screen_.width()-x;
	//apply corrections as needed
	if ((0 < scaleFactor_) && (0 <= offset_.x()) && (0 <= offset_.y())) {
		x = scaleFactor_*(x-offset_.x());
		y = scaleFactor_*(y-offset_.y());
	} else {
		qDebug() << "correction factors have not been initialized";
		rc = EXIT_FAILURE;
	}
	//threshold coordinates
	if (0 > x) x = 0.0;
	else if (screen_.width() < x) x = screen_.width()-10;
	if (0 > y) y = 0.0;
	else if (screen_.height() < y) y = screen_.height()-10;

	//convert back to integers
	pt.setX(x);
	pt.setY(y);

	return rc;
}

int GestureAlgos::filterKalman(QPointF &ptThumb, QPointF &ptIndex)
{
	static bool initDone = false;
	if (!initDone) {
		if (EXIT_FAILURE == initKalman()) {
			return EXIT_FAILURE;
		}
		initDone = true;
	}

	KF_.predict();
	measurement_(0) = static_cast<float>(ptThumb.x());
	measurement_(1) = static_cast<float>(ptThumb.y());
	measurement_(2) = static_cast<float>(ptIndex.x());
	measurement_(3) = static_cast<float>(ptIndex.y());
	cv::Mat estimated = KF_.correct(measurement_);
	qreal tmp = estimated.at<float>(0);
	ptThumb.setX(tmp);
	tmp = estimated.at<float>(1);
	ptThumb.setY(tmp);
	tmp = estimated.at<float>(2);
	ptIndex.setX(tmp);
	tmp = estimated.at<float>(3);
	ptIndex.setY(tmp);

	return EXIT_SUCCESS;
}

void GestureAlgos::initBiquad()
{
	sos_mat_[0][0] = 1.0, sos_mat_[0][1] = 1.0, sos_mat_[0][2] = 0.0, sos_mat_[0][3] = 1.0, 
		sos_mat_[0][4] = 0.1584, sos_mat_[0][5] = 0.0;
    sos_mat_[1][0] = 1.0, sos_mat_[1][1] = 2.0, sos_mat_[1][2] = 1.0, sos_mat_[1][3] = 1.0, 
		sos_mat_[1][4] = 0.3264, sos_mat_[1][5] = 0.0561;
    sos_mat_[2][0] = 1.0, sos_mat_[2][1] = 2.0, sos_mat_[2][2] = 1.0, sos_mat_[2][3] = 1.0,
		sos_mat_[2][4] = 0.3575, sos_mat_[2][5] = 0.1570;
    sos_mat_[3][0] = 1.0, sos_mat_[3][1] = 2.0, sos_mat_[3][2] = 1.0, sos_mat_[3][3] = 1.0,
		sos_mat_[3][4] = 0.4189, sos_mat_[3][5] = 0.3554;
    sos_mat_[4][0] = 1.0, sos_mat_[4][1] = 2.0, sos_mat_[4][2] = 1.0, sos_mat_[4][3] = 1.0,
		sos_mat_[4][4] = 0.5304, sos_mat_[4][5] = 0.7165;
	gain_ = 0.0189;
	for (int n = 0; n < SosMat::NB_BIQUADS; ++n) {
		biquadState[n].index = n;
		memset(biquadState[n].mem_in, 0, sizeof(biquadState[n].mem_in));
		memset(biquadState[n].mem_out, 0, sizeof(biquadState[n].mem_out));
	}
}

double GestureAlgos::biquad(BiquadState *state, double in)
{
	//filter output
	double out = sos_mat_[state->index][0]*in + 
		sos_mat_[state->index][1]*state->mem_in[0] + 
		sos_mat_[state->index][2]*state->mem_in[1] -
		sos_mat_[state->index][4]*state->mem_out[0] -
		sos_mat_[state->index][5]*state->mem_out[1];
	out = out/sos_mat_[state->index][3];
	//filter memory
	state->mem_in[1] = state->mem_in[0];
	state->mem_in[0] = in;
	state->mem_out[1] = state->mem_out[0];
	state->mem_out[0] = out;
	return out;
}

void GestureAlgos::filterLowPass(qreal &depth)
{
	static bool initDone = false;
	if (!initDone) {
		initBiquad();
		initDone = true;
	}
	//cascade of biquads
	for (int n = 0; n < SosMat::NB_BIQUADS; ++n) {
		depth = biquad(biquadState+n, depth);
	}
	depth = gain_*depth;
}

GestureAlgos::TouchType GestureAlgos::isTouch(qreal depthThumb, qreal depthIndex)
{
	//constants
	const static qreal DEPTH_THRESHOLD = 0.45;
	//permanent variable
	static GestureAlgos::TouchType out = GestureAlgos::TouchType::NONE;

	//make sure that the previous touch up is send only once
	if ((GestureAlgos::TouchType::DOUBLE_UP == out) ||
		(GestureAlgos::TouchType::SINGLE_UP == out)) {
			out = GestureAlgos::TouchType::NONE;
			return out;
	}

	//process depth information
	if ((DEPTH_THRESHOLD >= depthThumb) || (DEPTH_THRESHOLD >= depthIndex)) {
		//touch down
		out = ((DEPTH_THRESHOLD >= depthThumb) && (DEPTH_THRESHOLD >= depthIndex))?
			GestureAlgos::TouchType::DOUBLE_DOWN:GestureAlgos::TouchType::SINGLE_DOWN;
	} else {
		//touch up
		if (GestureAlgos::TouchType::DOUBLE_DOWN == out) {
			out = GestureAlgos::TouchType::DOUBLE_UP;
		} else if (GestureAlgos::TouchType::SINGLE_DOWN == out) {
			out = GestureAlgos::TouchType::SINGLE_UP;
		}
	}

	return out;
}
