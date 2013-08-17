#include <QPoint>
#include <QDebug>
#include "TouchInputEmulator.h"

TouchInputEmulator::TouchInputEmulator() : update_(false)
{
	if (TRUE == InitializeTouchInjection(MAX_NB_CONTACT_POINTS, TOUCH_FEEDBACK_DEFAULT)) {
		ready_ = true;
	} else {
		qDebug() << "error in InitializeTouchInjection " << GetLastError();
	}
}

void TouchInputEmulator::initContact(const QPoint &pt, int id)
{
	memset(contact_+id, 0, sizeof(POINTER_TOUCH_INFO)); 
	contact_[id].pointerInfo.pointerType = PT_TOUCH;
	contact_[id].pointerInfo.pointerId = id;          //contact index
	contact_[id].pointerInfo.ptPixelLocation.y = pt.y(); // Y co-ordinate of touch on screen
	contact_[id].pointerInfo.ptPixelLocation.x = pt.x(); // X co-ordinate of touch on screen

	contact_[id].touchFlags = TOUCH_FLAG_NONE;
	contact_[id].touchMask = TOUCH_MASK_CONTACTAREA | TOUCH_MASK_ORIENTATION | TOUCH_MASK_PRESSURE;
	contact_[id].orientation = 90; // Orientation of 90 means touching perpendicular to screen.
	contact_[id].pressure = 32000; 

	// defining contact area (4 x 4 pixels)
	contact_[id].rcContact.top = contact_[id].pointerInfo.ptPixelLocation.y - 2;
	contact_[id].rcContact.bottom = contact_[id].pointerInfo.ptPixelLocation.y + 2;
	contact_[id].rcContact.left = contact_[id].pointerInfo.ptPixelLocation.x  - 2;
	contact_[id].rcContact.right = contact_[id].pointerInfo.ptPixelLocation.x  + 2;
}

int TouchInputEmulator::touchDown(const QPoint &pt)
{
	if (!ready_) { // Here number of contact point is declared as 1.
		qDebug() << "Error in InitializeTouchInjection";
		return EXIT_FAILURE;
	}
	
	initContact(pt, 0);
	contact_[0].pointerInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
	if (update_) {
		contact_[0].pointerInfo.pointerFlags |= POINTER_FLAG_UPDATE;
	} else {
		contact_[0].pointerInfo.pointerFlags |= POINTER_FLAG_DOWN;
		update_ = true;
	}
	if (FALSE == InjectTouchInput(1, contact_)) { // Injecting the touch down on screen
		qDebug() << "Error in InjectTouchInput " << GetLastError();
		return EXIT_FAILURE;
	}
	qDebug() << "single touch down at" << pt;
	return EXIT_SUCCESS;
}

int TouchInputEmulator::touchUp(const QPoint &pt)
{
	if (!ready_) { // Here number of contact point is declared as 1.
		qDebug() << "Error in InitializeTouchInjection";
		return EXIT_FAILURE;
	}	

	initContact(pt, 0);
	contact_[0].pointerInfo.pointerFlags = POINTER_FLAG_UP;
	update_ = false;
	if (FALSE == InjectTouchInput(1, contact_)) { // Injecting the touch up from screen
		qDebug() << "Error in InjectTouchInput " << GetLastError();
		return EXIT_FAILURE;
	}	
	qDebug() << "single touch up at" << pt;
	return EXIT_SUCCESS;
}

int TouchInputEmulator::touchDown(const QPoint &pt1, const QPoint &pt2)
{
	if (!ready_) { // Here number of contact point is declared as 1.
		qDebug() << "Error in InitializeTouchInjection";
		return EXIT_FAILURE;
	}
	initContact(pt1, 0);
	initContact(pt2, 1);
	for (int i = 0; i < MAX_NB_CONTACT_POINTS; ++i) {
		contact_[i].pointerInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
		if (update_) {
			contact_[i].pointerInfo.pointerFlags |= POINTER_FLAG_UPDATE;
		} else {
			contact_[i].pointerInfo.pointerFlags |= POINTER_FLAG_DOWN;
			if ((MAX_NB_CONTACT_POINTS-1) == i) update_ = true;
		}
	}
	if (FALSE == InjectTouchInput(2, contact_)) { // Injecting the touch down from screen
		qDebug() << "Error in InjectTouchInput " << GetLastError();
		return EXIT_FAILURE;
	}
	qDebug() << "double touch down at" << pt1 << "," << pt2;
	return EXIT_SUCCESS;
}

int TouchInputEmulator::touchUp(const QPoint &pt1, const QPoint &pt2)
{
	if (!ready_) { // Here number of contact point is declared as 1.
		qDebug() << "Error in InitializeTouchInjection";
		return EXIT_FAILURE;
	}
	initContact(pt1, 0);
	initContact(pt2, 1);
	for (int i = 0; i < 2; ++i) {
		contact_[i].pointerInfo.pointerFlags = POINTER_FLAG_UP;
	}
	update_ = false;
	if (FALSE == InjectTouchInput(2, contact_)) { // Injecting the touch up from screen
		qDebug() << "Error in InjectTouchInput " << GetLastError();
		return EXIT_FAILURE;
	}	
	qDebug() << "double touch up at" << pt1 << "," << pt2;
	return EXIT_SUCCESS;
}
