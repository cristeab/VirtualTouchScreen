#include <QPoint>
#include <QDebug>
#include "TouchInputEmulator.h"

TouchInputEmulator::TouchInputEmulator()
{
	ready_ = (TRUE == InitializeTouchInjection(MAX_NB_CONTACT_POINTS, TOUCH_FEEDBACK_DEFAULT));
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
	contact_[0].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
	if (FALSE == InjectTouchInput(1, contact_)) { // Injecting the touch down on screen
		qDebug() << "Error in InjectTouchInput";
		return EXIT_FAILURE;
	}
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
	if (FALSE == InjectTouchInput(1, contact_)) { // Injecting the touch up from screen
		qDebug() << "Error in InjectTouchInput";
		return EXIT_FAILURE;
	}
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
	for (int i = 0; i < 2; ++i) {
		contact_[i].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
	}
	if (FALSE == InjectTouchInput(2, contact_)) { // Injecting the touch down from screen
		qDebug() << "Error in InjectTouchInput";
		return EXIT_FAILURE;
	}
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
	if (FALSE == InjectTouchInput(2, contact_)) { // Injecting the touch up from screen
		qDebug() << "Error in InjectTouchInput";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
