#ifndef PresenterHelper_H
#define PresenterHelper_H

#include <QtWidgets/QMainWindow>
#include <Windows.h>
#include "util_pipeline.h"

class GestureThread;
class GestureAlgos;
class ConfigDialog;

class PresenterHelper : public QMainWindow
{
	Q_OBJECT
	friend class GestureThread;
	friend class ConfigDialog;
public:
	explicit PresenterHelper(QWidget *parent = 0);
	~PresenterHelper();
	enum {OFFSET_X = 200, OFFSET_Y = 100, 
		SCALE_FACTOR_x100 = 100,
		NB_SWIPES_PER_PAGE_SWITCH = 1,
		POINTER_SIZE = 15};

public slots:
	void showMenu();
	void showHelp();
	void onMoveCursor(int x, int y);
	void onTap(int x, int y);
	void onShowCoords(int x, int y);
	void onSwipe(BYTE code);

private:
	void setupActions();
	void loadPointer(const QString &path, int size);
	void loadSettings();
	void saveSettings();
	GestureThread *gestureThread;
	GestureAlgos *gestureAlgos;
	pxcU32 imgWidth;
	pxcU32 imgHeight;
	QString pointerIconPath;
	int pointerSize;
	int offsetX;
	int offsetY;
	double scaleFactor;
	ConfigDialog *config;
};

#endif // PresenterHelper_H
