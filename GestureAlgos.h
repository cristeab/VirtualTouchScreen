#pragma once

#include "opencv2/video/tracking.hpp"

class GestureAlgos
{
public:
	GestureAlgos();
	static GestureAlgos* instance() {
		if (NULL == instance_) {
			instance_ = new GestureAlgos();
		}
		return instance_;
	}
	~GestureAlgos() {
		delete instance_;
	}
	void setScreenSize(int scrWidth, int scrHeight) {
		scrWidth_ = scrWidth;
		scrHeight_ = scrHeight;
	}
	void setImageSize(int imgWidth, int imgHeight) {
		imgWidth_ = imgWidth;
		imgHeight_ = imgHeight;
	}
	void setCorrectionFactors(float scaleFactor, int offsetX, int offsetY) {
		scaleFactor_ = scaleFactor;
		offsetX_ = offsetX;
		offsetY_ = offsetY;
	}
	int filterKalman(float &x, float &y);
	bool isTap(double samp);
	bool isPressAndHold(int x, int y, double depth);
private:
	int initKalman();
	int imageToScreen(float &x, float &y);
	enum {MAX_OBS_DURATION = 15, MAX_NB_SAMPLES = 2};
	int scrWidth_;
	int scrHeight_;
	int imgWidth_;
	int imgHeight_;
	float scaleFactor_;
	int offsetX_;
	int offsetY_;
	cv::KalmanFilter KF_;
	cv::Mat_<float> measurement_;
	static GestureAlgos *instance_;
	GestureAlgos(const GestureAlgos&);
	GestureAlgos& operator=(const GestureAlgos&);
};