#include <QDebug>
#include <QDesktopWidget>
#include <QBitmap>
#include <QMessageBox>
#include <QApplication>
#include <QAction>
#include <QSettings>
#include <QPainter>
#include "GestureThread.h"
#include "GestureAlgos.h"
#include "VirtualTouchScreen.h"
#include "TouchInputEmulator.h"
//#include "ConfigDialog.h"

#define APPLICATION_NAME "Virtual Touch Screen"

VirtualTouchScreen::VirtualTouchScreen(QWidget *parent)
	: QMainWindow(parent),
	gestureThread(NULL),
	offset(OFFSET_X,OFFSET_Y),
	scaleFactor(SCALE_FACTOR_x100/100.0),
	thumbPointer(new QWidget()),
	config(NULL),
	handSkeletonPoints_(Hand::POINTS),
	touch_(new TouchInputEmulator()),
	virtualScreenThreshold_(0.35)
{
	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	thumbPointer->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Popup);
	//load finger icons
	setFingerPointer(this, ":/icons/fingerprint.png", 64);
	setFingerPointer(thumbPointer, ":/icons/fingerprint.png", 64, true);
	thumbPointer->show();

	loadSettings();

	//get screen size
	QDesktopWidget *desktop = QApplication::desktop();
	QRect geom = desktop->availableGeometry(0);//first screen

	//init gesture algorithms
	gestureAlgos = GestureAlgos::instance();
	gestureAlgos->setScreenSize(QSize(geom.width(), geom.height()));
	gestureAlgos->setCorrectionFactors(scaleFactor, offset);
	gestureAlgos->setMainWindow(this);

	//config = new ConfigDialog(NULL, this);//screen size must be set

	setupActions();

	qDebug() << QThread::currentThreadId() << "starting gesture thread";
	gestureThread = new GestureThread(this);
	gestureThread->start();
}

VirtualTouchScreen::~VirtualTouchScreen()
{
	gestureThread->terminate();
	gestureThread->wait();
	delete gestureThread;
	delete touch_;
	delete thumbPointer;
	delete config;
	saveSettings();
}

void VirtualTouchScreen::setFingerPointer(QWidget *target, 
										  const QString &iconPath, 
										  int iconSize, bool rotate)
{
	QPixmap pix(iconPath);
	if ((iconSize != pix.size().width()) && (0 < iconSize)) {
		pix = pix.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	if (rotate) {
		QImage orig = pix.toImage();
		pix = QPixmap::fromImage(rotate270(orig));
	}
	if (!pix.isNull()) {
		QPalette p = palette();
		p.setBrush(QPalette::Background, pix);
		target->setPalette(p);
		target->resize(pix.size());
		target->setMask(pix.mask());
	}
	else {
		qDebug() << "Cannot load cursor pixmap";
	}
}

QImage VirtualTouchScreen::rotate270(const QImage &src) {
    QImage dst(src.height(), src.width(), src.format());
    for (int y=0;y<src.height();++y) {
        const uint *srcLine = reinterpret_cast< const uint * >(src.scanLine(y));
        for (int x=0;x<src.width();++x) {
            dst.setPixel(y, src.width()-x-1, srcLine[x]);
        }
    }
    return dst;
}

void VirtualTouchScreen::setupActions()
{
	QAction *menuAction = new QAction(this);
	menuAction->setShortcut(QKeySequence("F2"));
	connect(menuAction, SIGNAL(triggered(bool)), this, SLOT(showMenu()));
	addAction(menuAction);

	QAction *quitAction = new QAction(this);
	quitAction->setShortcut(QKeySequence("Ctrl+q"));
	connect(quitAction, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
	addAction(quitAction);

	QAction *helpAction = new QAction(this);
	helpAction->setShortcut(QKeySequence("F1"));
	connect(helpAction, SIGNAL(triggered(bool)), this, SLOT(showHelp()));
	addAction(helpAction);

	QAction *hideAction = new QAction(this);
	hideAction->setShortcut(QKeySequence("F3"));
	connect(hideAction, SIGNAL(triggered(bool)), this, SLOT(showMinimized()));
	addAction(hideAction);
}

void VirtualTouchScreen::showMenu()
{
	if (NULL != config)
	{
		//config->show();
	}
}

void VirtualTouchScreen::showHelp()
{
	QMessageBox::about(NULL, APPLICATION_NAME, 
		"                            Presenter Helper\n\n"
		"                        author: Bogdan Cristea\n"
		"                        e-mail: cristeab@gmail.com\n\n"
		"   Presenter Helper aims at replacing the laser pointer used in presentations "
		"by a Creative Interactive Gesture Camera, so that a pointer can be moved on "
		"the screen while the presenter moves his hand in front of the camera.\n"
		"   Any presentation software can be used as long as slides are changed using "
		"left/right arrow keys. Optionally animations can be activated with the "
		"mouse left click.\n"
		"   The presenter changes the slides using left/right swipes and a single tap "
		"can be used to either activate presentation animations or can be assigned "
		"as left swipe. If needed, the pointer can be completely hidden. "
		"In order to send the key or mouse events to the presentation application, make "
		"sure that the presentation application has the focus during presentations.\n"
		"   The pointer icon (*.jpg and *.png image formats are supported) and "
		"its size are configurable, also the coordinates of the "
		"upper-left corner of the area covered by the pointer. A scaling factor can "
		"be used in order to make sure that the entire screen area is covered by the "
		"pointer. The number of swipes needed to activate a page switch can be changed, "
		"the default value is one, a value of zero disables the swipe gesture, while "
		"a value superior to one can be used if the slide switch happens too often "
		"during presentation. By default pointer coordinates are filtered with a "
		"Kalman filter that can be disabled if needed.\n"
		"   The following shortcut keys are available:\n"
		"- F1: shows this help\n"
		"- F2: shows the configuration dialog\n"
		"- F3: minimizes the pointer\n"
		"- Ctrl+q: quits the application\n"
		"In order to receive the shortcut keys the application needs to have "
		"the focus (see above). In order to have the focus either click on "
		"the application icon on the main toolbar or click the pointer.");
}

void VirtualTouchScreen::onMoveHand()
{
	QMutexLocker lock(&skeletonPointMutex_);
	QSize size = this->size();
	move(handSkeletonPoints_[INDEX].x()-size.width()/2, 
		handSkeletonPoints_[INDEX].y()-size.height()/2);
	if (NULL != thumbPointer) {
		size = thumbPointer->size();
		thumbPointer->move(handSkeletonPoints_[THUMB].x()-size.width()/2, 
			handSkeletonPoints_[THUMB].y()-size.height()/2);
	}
}

void VirtualTouchScreen::onShowThumb(bool visible)
{
	if (NULL != thumbPointer) {
		thumbPointer->setVisible(visible);
	}
}

void VirtualTouchScreen::onSwipe(BYTE code)
{
	keybd_event(code & 0xff, 0, 0, 0);
	keybd_event(code & 0xff, 0, KEYEVENTF_KEYUP, 0);
}

void VirtualTouchScreen::onTouchDown(const QPoint &ptThumb, const QPoint &ptIndex)
{
	if ((NULL != touch_) && (EXIT_FAILURE == touch_->touchDown(ptThumb, ptIndex))) {
		qDebug() << "Cannot send double touch down";
	}
}

void VirtualTouchScreen::onTouchDown(const QPoint &ptIndex)
{
	if ((NULL != touch_) && (EXIT_FAILURE == touch_->touchDown(ptIndex))) {
		qDebug() << "Cannot send single touch down";
	}
}

void VirtualTouchScreen::onTouchUp(const QPoint &ptThumb, const QPoint &ptIndex)
{
	if ((NULL != touch_) && (EXIT_FAILURE == touch_->touchUp(ptThumb, ptIndex))) {
		qDebug() << "Cannot send double touch up";
	}
}

void VirtualTouchScreen::onTouchUp(const QPoint &ptIndex)
{
	if ((NULL != touch_) && (EXIT_FAILURE == touch_->touchUp(ptIndex))) {
		qDebug() << "Cannot send single touch up";
	}
}

#define COMPANY_NAME "Bogdan Cristea"
#define POINTER_ICON_PATH "PointerIconPath"
#define KEY_POINTER_SIZE "PointerSize"
#define KEY_OFFSET_X "OffsetX"
#define KEY_OFFSET_Y "OffsetY"
#define SCALE_FACTOR "ScaleFactor"
#define NB_SWIPES "NbSwipes"
#define TAP_FOR_FORWARD_SWITCH "TapForForwardSwitch"
#define USE_KALMAN_FILTER "UseKalmanFilter"

void VirtualTouchScreen::loadSettings()
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	pointerIconPath = settings.value(POINTER_ICON_PATH, ":/icons/green-pointer.png").toString();
	offset.setX(settings.value(KEY_OFFSET_X, OFFSET_X).toInt());
	offset.setY(settings.value(KEY_OFFSET_Y, OFFSET_Y).toInt());
	scaleFactor = settings.value(SCALE_FACTOR, SCALE_FACTOR_x100/100.0).toDouble();
}

void VirtualTouchScreen::saveSettings()
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	settings.setValue(POINTER_ICON_PATH, pointerIconPath);
	settings.setValue(KEY_OFFSET_X, offset.x());
	settings.setValue(KEY_OFFSET_Y, offset.y());
	settings.setValue(SCALE_FACTOR, scaleFactor);
}
