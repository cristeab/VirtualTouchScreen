#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QDebug>
#include <QPainter>
#include "VirtualTouchScreen.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	QPixmap splashImg(":/icons/splash.jpg");
	if (!splashImg.isNull())
	{
		QPainter painter(&splashImg);
		painter.setPen(Qt::darkBlue);
		QFont font("Arial", 30);
		painter.setFont(font);
		int posX = 40;
		int posY = 250;
		painter.drawText(posX, posY, "Virtual Touch Screen");
		painter.setPen(Qt::blue);
		painter.setFont(QFont("Arial", 12, -1, true));
		posY = 300;
		posX = 150;
		painter.drawText(posX, posY+150, "Copyright Bogdan Cristea");
		painter.drawText(posX-10, posY+170, "e-mail: cristeab@gmail.com");
	}
	else
	{
		qDebug() << "Cannot load splash screen";
	}

	QSplashScreen splashScr(splashImg);
	splashScr.show();
	splashScr.showMessage("Initializing gesture camera ...");
	a.processEvents();

	PresenterHelper w;
	w.show();

	splashScr.finish(&w);

	return a.exec();
}
