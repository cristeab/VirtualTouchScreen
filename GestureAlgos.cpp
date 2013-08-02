#include <qglobal.h>
#include "GestureAlgos.h"

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

bool GestureAlgos::isPressAndHold(double depth)
{
	return false;
}
