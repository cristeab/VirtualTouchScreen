#pragma once

#include <QPoint>
#include <QSize>
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
	void setScreenSize(const QSize &screen) {
		screen_ = screen;
	}
	void setImageSize(const QSize &image) {
		image_ = image;
	}
	void setCorrectionFactors(float scaleFactor, const QPoint &offset) {
		scaleFactor_ = scaleFactor;
		offset_ = offset;
	}
	//transforms to screen coordinates
	int imageToScreen(QPoint &pt);
	//filters
	int filterKalman(QPoint &pt);
	void filterLowPass(float &depth);
	template<typename T>
	void filterDiff(T &depth, T &diffState);
	//gestures
	bool isTap(const QPoint &pt, float depth);
	bool isPressAndHold(const QPoint &pt, float depth);
	bool isSlide(const QPoint &pt, float depth);
	bool isPinch(const QPoint &pt, float depth);
	bool isStretch(const QPoint &pt, float depth);
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
	QSize screen_;
	QSize image_;
	float scaleFactor_;
	QPoint offset_;
	cv::KalmanFilter KF_;
	cv::Mat_<float> measurement_;
	GestureAlgos(const GestureAlgos&);
	GestureAlgos& operator=(const GestureAlgos&);
	~GestureAlgos() {};
};