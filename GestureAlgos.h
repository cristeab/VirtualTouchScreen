#pragma once

#include <QPointF>
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
	QPointF imageCenter() const {
		return QPointF(image_.width()/2.0, image_.height()/2.0);
	}
	QSize imageSize() const {
		return image_;
	}
	void setCorrectionFactors(qreal scaleFactor, const QPointF &offset) {
		scaleFactor_ = scaleFactor;
		offset_ = offset;
	}
	//transforms to screen coordinates
	int imageToScreen(QPointF &pt);
	//transforms absolute finger coordinates to coordinates relative to the center of the hand which is 
	//at the fixed position (imgW/2, imgH/2)
	int toHandCenter(QPointF &pt, const QPointF &handPos);
	//filters
	int filterKalman(QPointF &ptThumb, QPointF &ptIndex);
	void filterLowPass(qreal &depth);
	//detect when the hand touches the virtual touch screen
	//the OS decides which gesture has been made
	enum TouchType {NONE = 0, SINGLE_DOWN, SINGLE_UP, DOUBLE_DOWN, DOUBLE_UP};
	TouchType isTouch(qreal depthThumb, qreal depthIndex);
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
	qreal scaleFactor_;
	QPointF offset_;
	cv::KalmanFilter KF_;
	cv::Mat_<float> measurement_;
	GestureAlgos(const GestureAlgos&);
	GestureAlgos& operator=(const GestureAlgos&);
	~GestureAlgos() {};
};