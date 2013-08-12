#pragma once

#include <Windows.h>
class QPoint;

class TouchInputEmulator {
public:
	TouchInputEmulator();
	//single point touch
	int touchDown(const QPoint &pt);
	int touchUp(const QPoint &pt);
	//two point touch
	int touchDown(const QPoint &pt1, const QPoint &pt2);
	int touchUp(const QPoint &pt1, const QPoint &pt2);
private:
	void initContact(const QPoint &pt, int id);
	enum {MAX_NB_CONTACT_POINTS = 2};
	bool ready_;
	POINTER_TOUCH_INFO contact_[MAX_NB_CONTACT_POINTS];
};