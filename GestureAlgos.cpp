#include <qglobal.h>
#include <QDebug>
#include "GestureAlgos.h"

GestureAlgos::GestureAlgos() :
	scrWidth_(0), scrHeight_(0),
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
		qDebug() << "either image width or image height is not initialized";
		rc = EXIT_FAILURE;
	}
	//invert X axis
	x = scrWidth_-x;
	//apply corrections as needed
	if ((0 < scaleFactor_) && (0 <= offsetX_) && (0 <= offsetY_)) {
		x = scaleFactor_*(x-offsetX_);
		y = scaleFactor_*(y-offsetY_);
	} else {
		qDebug() << "correction factors have not been initialized";
		rc = EXIT_FAILURE;
	}
	//threshold coordinates
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

	KF_.predict();
	measurement_(0) = x;
	measurement_(1) = y;
	cv::Mat estimated = KF_.correct(measurement_);
	x = static_cast<int>(estimated.at<float>(0));
	y = static_cast<int>(estimated.at<float>(1));

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

void GestureAlgos::filterLowPass(float &depth)
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
	depth = static_cast<float>(gain_*depth);
}

template<typename T>
void GestureAlgos::filterDiff(T &depth, T &prevDepth)
{
	T tmp = depth;
	depth -= prevDepth;
	prevDepth = tmp;
}

bool GestureAlgos::isTap(int x, int y, float depth)
{
	//constants
	const static float DIFF_DEPTH_THRESHOLD = static_cast<float>(0.02);
	const static int DIFF_POS_THRESHOLD = 20;
	const static int MAX_OBS_DURATION = 20;
	//permanent variables
	static int obsDuration = 0;
	static int varSign = 0;
	static float prevDepth = static_cast<float>(0);
	static int prevX = 0;
	static int prevY = 0;

	bool out = false;

	filterDiff(depth, prevDepth);
	if (qAbs(depth) > DIFF_DEPTH_THRESHOLD)
	{
		int sign = static_cast<int>((depth)/qAbs(depth));
		if (0 == varSign)
		{
			//variation detected
			varSign = sign;
			obsDuration = 0;
		}
		else if (varSign != sign)
		{
			//sign change detected
			if (-1 == varSign) {
				out = true;
				varSign = 0;
			} else {
				varSign = -varSign;
				obsDuration = 0;
			}
		}
	}

	//check the position of the hand (helps reducing the false alarms)
	filterDiff(x, prevX);
	filterDiff(y, prevY);
	if ((qAbs(x) > DIFF_POS_THRESHOLD) || (qAbs(y) > DIFF_POS_THRESHOLD)) {
		varSign = 0;//not a tap
	}

	//increment the observation duration once the hand is closer to the camera
	if (-1 == varSign)
	{
		++obsDuration;
	}
	//tap duration cannot exceed the maximum observation duration
	if (MAX_OBS_DURATION < obsDuration)
	{
		varSign = 0;
	}

	return out;
}

bool GestureAlgos::isPressAndHold(int x, int y, float depth)
{
	//constants
	const static float DIFF_DEPTH_THRESHOLD = static_cast<float>(0.02);
	const static int DIFF_POS_THRESHOLD = 20;
	const static int MAX_OBS_DURATION = 50;
	//permanent variables
	static int obsDuration = 0;
	static int varSign = 0;
	static float prevDepth = static_cast<float>(0);
	static int prevX = 0;
	static int prevY = 0;

	bool out = false;

	filterDiff(depth, prevDepth);
	if (qAbs(depth) > DIFF_DEPTH_THRESHOLD)
	{
		int sign = static_cast<int>((depth)/qAbs(depth));
		if (0 == varSign)
		{
			//variation detected
			varSign = sign;
			obsDuration = 0;
		}
		else if (varSign != sign)
		{
			//sign change detected
			varSign = -varSign;
			obsDuration = 0;
		}
	}

	//check the position of the hand (helps reducing the false alarms)
	filterDiff(x, prevX);
	filterDiff(y, prevY);
	if ((qAbs(x) > DIFF_POS_THRESHOLD) || (qAbs(y) > DIFF_POS_THRESHOLD)) {
		varSign = 0;//not a tap
	}

	if (-1 == varSign)
	{
		//press detected
		++obsDuration;
		if (MAX_OBS_DURATION < obsDuration)
		{
			out = true;
			varSign = 0;
		}
	}

	return out;
}

bool GestureAlgos::isSlide(int x, int y, float depth)
{
	//constants
	const static float DIFF_DEPTH_THRESHOLD = static_cast<float>(0.015);
	const static int DIFF_POS_THRESHOLD = 20;
	const static int MAX_OBS_DURATION = 35;
	//permanent variables
	static int obsDuration = 0;
	static bool slideDetected = false;
	static int varSign = 0;
	static float prevDepth = static_cast<float>(0);
	static int prevX = 0;
	static int prevY = 0;

	bool out = false;

	filterDiff(depth, prevDepth);
	if (qAbs(depth) > DIFF_DEPTH_THRESHOLD)
	{
		int sign = static_cast<int>((depth)/qAbs(depth));
		if (0 == varSign)
		{
			//variation detected
			varSign = sign;
			obsDuration = 0;
			slideDetected = false;
		}
		else if (varSign != sign)
		{
			//sign change detected
			varSign = -varSign;
			obsDuration = 0;
			slideDetected = false;
		}
	}

	//check the position of the hand (helps reducing the false alarms)
	filterDiff(x, prevX);
	filterDiff(y, prevY);
	if ((qAbs(x) > DIFF_POS_THRESHOLD) || (qAbs(y) > DIFF_POS_THRESHOLD)) {
		slideDetected = true;
	}

	if (-1 == varSign)
	{
		//press detected
		++obsDuration;
		if ((MAX_OBS_DURATION < obsDuration) && slideDetected)
		{
			out = true;
			varSign = 0;
		}
	}

	return out;
}

bool GestureAlgos::isPinch(int x, int y, float depth)
{
	return false;
}

bool GestureAlgos::isStretch(int x, int y, float depth)
{
	return false;
}
