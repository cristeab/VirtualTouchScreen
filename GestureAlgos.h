#pragma once

#include "opencv2/video/tracking.hpp"

class GestureAlgos
{
public:
	GestureAlgos();
	static GestureAlgos* instance() {
		static GestureAlgos instance;
		return &instance;
	}
	//algorithms parameters
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
	//transforms to screen coordinates
	int imageToScreen(float &x, float &y);
	//filters
	int filterKalman(float &x, float &y);
	void filterBiquad(float &depth);
	template<typename T>
	void filterDiff(T &depth, T &diffState);
	//gestures
	bool isTap(int x, int y, float depth);
	bool isPressAndHold(int x, int y, float depth);
	bool isSlide(int x, int y, float depth);
	bool isPinch(int x, int y, float depth);
	bool isStretch(int x, int y, float depth);
	//swipe, swipe from edge and turn are implemented in GestureThread
private:
	int initKalman();
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
	int scrWidth_;
	int scrHeight_;
	int imgWidth_;
	int imgHeight_;
	float scaleFactor_;
	int offsetX_;
	int offsetY_;
	cv::KalmanFilter KF_;
	cv::Mat_<float> measurement_;
	GestureAlgos(const GestureAlgos&);
	GestureAlgos& operator=(const GestureAlgos&);
	~GestureAlgos() {};
};