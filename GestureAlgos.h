#pragma once

class GestureAlgos
{
public:
	static bool isTap(double samp);
	static bool isPressAndHold(double depth);
private:
	enum {MAX_OBS_DURATION = 15, MAX_NB_SAMPLES = 2};
};