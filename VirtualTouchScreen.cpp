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
#include "ConfigDialog.h"

#define APPLICATION_NAME "Virtual Touch Screen"
#define FINGER_DEFAULT_ICON ":/icons/fingerprint.png"

VirtualTouchScreen::VirtualTouchScreen(QWidget *parent)
	: QMainWindow(parent),
	gestureThread(NULL),
	offset(OFFSET_X,OFFSET_Y),
	scaleFactor(SCALE_FACTOR_x100/100.0),
	thumbPointer(new QWidget()),
	config(NULL),
	handSkeletonPoints_(Hand::POINTS),
	touch_(new TouchInputEmulator()),
	virtualScreenThreshold_(VIRTUAL_SCREEN_THRESHOLD_x100/100.0),
	fingerIcon_(FINGER_DEFAULT_ICON),
	fingerIconSize_(FINGER_ICON_SIZE),
	hideThumb_(false)
{
	loadSettings();

	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	thumbPointer->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
	loadFingerIcons(fingerIcon_, fingerIconSize_);
	if (!hideThumb_) thumbPointer->show();	

	//get screen size
	QDesktopWidget *desktop = QApplication::desktop();
	QRect geom = desktop->availableGeometry(0);//first screen

	//init gesture algorithms
	gestureAlgos = GestureAlgos::instance();
	gestureAlgos->setScreenSize(QSize(geom.width(), geom.height()));
	gestureAlgos->setCorrectionFactors(scaleFactor, offset);
	gestureAlgos->setMainWindow(this);

	setupActions();

	config = new ConfigDialog(this);

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

void VirtualTouchScreen::loadFingerIcons(const QString &iconPath, int iconSize)
{
	setFingerPointer(this, iconPath, iconSize);
	setFingerPointer(thumbPointer, iconPath, iconSize, true);	
}

QImage VirtualTouchScreen::rotate270(const QImage &src)
{
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
}

void VirtualTouchScreen::showMenu()
{
	if (NULL != config)
	{
		thumbPointer->hide();
		config->show();
	}
}

void VirtualTouchScreen::showHelp()
{
	QMessageBox::about(NULL, APPLICATION_NAME, 
		"                                        Virtual Touch Screen\n\n"
		"                                      author: Bogdan Cristea\n"
		"                                    e-mail: cristeab@gmail.com\n\n"
		"  This application aims at improving user experience by providing a touch screen "
		"experience with gestures. Currently touch screen enabled-applications have become "
		"increasingly popular. However, classical PCs cannot benefit from this new type of "
		"input. This application tries to fill this gap by providing touch screen experience "
		"on any type of PC/device using a Creative Interactive Gesture Camera and perceptual "
		"computing.\n"
		"   The virtual touch screen is represented by the sphere centered on the camera "
		"and of radius equal with a virtual touch screen threshold. Once the fingers are "
		"on the sphere or inside a touch event is generated. Since both the thumb and the "
		"index fingers are tracked, in theory, all eight gestures supported in Windows 8 "
		"are available:\n"
		" - Press and Hold\n"
		" - Tap\n"
		" - Slide\n"
		" - Swipe\n"
		" - Pinch\n"
		" - Stretch\n"
		" - Swipe from Edge\n"
		" - Turn\n"
		"However, in practice, with a single camera, the distance estimation has fluctuations "
		"that might have a negative impact on virtual touch screen accuracy.\n"
		"   The virtual touch screen threshold is configurable through the configuration "
		"dialog. Finger icon (*.jpg and *.png image formats are supported) and "
		"its size can also be changed. Both the thumb and the index fingers use the same "
		"icon, but the thumb finger uses the icon rotated with 90 degrees counterclock wise. "
		"If needed, the thumb finger icon can be hidden while the touch events from the thumb "
		"are still received. The coordinates of the upper-left corner of the area covered "
		"by the fingers can be changed, also the scaling factor used to make sure that "
		"the entire screen area is covered.\n"
		"   The following shortcut keys are available:\n"
		"- F1: shows this help\n"
		"- F2: shows the configuration dialog\n"
		"- Ctrl+q: quits the application\n"
		"In order to receive the shortcut keys the application needs to have "
		"the focus. In order to have the focus either click on "
		"the application icon on the main toolbar or click the pointer.");
}

void VirtualTouchScreen::onMoveHand()
{
	QMutexLocker lock(&skeletonPointMutex_);
	QSize size = this->size();
	move(handSkeletonPoints_[INDEX].x()-size.width()/2, 
		handSkeletonPoints_[INDEX].y()-size.height()/2);
	if ((NULL != thumbPointer) && (!hideThumb_)) {
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
#define KEY_VIRTUAL_SCREEN_THRESHOLD "VirtualScreenThreshold"
#define KEY_FINGER_ICON "FingerIcon"
#define KEY_FINGER_ICON_SIZE "FingerIconSize"
#define KEY_OFFSET_X "OffsetX"
#define KEY_OFFSET_Y "OffsetY"
#define SCALE_FACTOR "ScaleFactor"
#define KEY_HIDE_THUMB "HideThumb"

void VirtualTouchScreen::loadSettings()
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	virtualScreenThreshold_ = settings.value(KEY_VIRTUAL_SCREEN_THRESHOLD, 
		VIRTUAL_SCREEN_THRESHOLD_x100/100.0).toDouble();
	fingerIcon_ = settings.value(KEY_FINGER_ICON, FINGER_DEFAULT_ICON).toString();
	fingerIconSize_ = settings.value(KEY_FINGER_ICON_SIZE, FINGER_ICON_SIZE).toInt();
	hideThumb_ = settings.value(KEY_HIDE_THUMB, false).toBool();
	offset.setX(settings.value(KEY_OFFSET_X, OFFSET_X).toInt());
	offset.setY(settings.value(KEY_OFFSET_Y, OFFSET_Y).toInt());
	scaleFactor = settings.value(SCALE_FACTOR, SCALE_FACTOR_x100/100.0).toDouble();
}

void VirtualTouchScreen::saveSettings()
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	settings.setValue(KEY_VIRTUAL_SCREEN_THRESHOLD, virtualScreenThreshold_);
	settings.setValue(KEY_FINGER_ICON, fingerIcon_);
	settings.setValue(KEY_FINGER_ICON_SIZE, fingerIconSize_);
	settings.setValue(KEY_HIDE_THUMB, hideThumb_);
	settings.setValue(KEY_OFFSET_X, offset.x());
	settings.setValue(KEY_OFFSET_Y, offset.y());
	settings.setValue(SCALE_FACTOR, scaleFactor);
}
