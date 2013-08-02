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
	//used to filter hand coordinates
	int filterKalman(float &x, float &y);
	//used to filter depth information
	int filterBiquad(float &depth);
	//gestures
	bool isTap(int x, int y, float depth);
	bool isPressAndHold(int x, int y, float depth);
	bool isSlide(int x, int y, float depth);
	bool isPinch(int x, int y, float depth);
	bool isStretch(int x, int y, float depth);
	//swipe, swipe from edge and turn are implemented in GestureThread
private:
	int initKalman();
	int imageToScreen(float &x, float &y);
	struct BiquadState {
		unsigned int index;
		double mem_in[2];
		double mem_out[2];
	};
	double biquad(BiquadState *state, double in);
	void initBiquad();
	enum SosMat {NB_BIQUADS = 5};
	double sos_mat_[SosMat::NB_BIQUADS][6];
	double gain_;
	BiquadState biquadState[SosMat::NB_BIQUADS];
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