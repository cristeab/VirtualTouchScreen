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
		QSize size = splashImg.size();
		int posX = size.width()/6;
		int posY = size.height()/2;
		painter.drawText(posX, posY, "Virtual Touch Screen");
		painter.setPen(Qt::blue);
		painter.setFont(QFont("Arial", 15));
		posY -= 30;
		painter.drawText(posX, posY+100, "Available commands:");
		painter.drawText(posX, posY+130, " - F1: help");
		painter.drawText(posX, posY+160, " - F2: shows the configuration dialog");
		painter.drawText(posX, posY+190, " - ctrl+q: quits the application");
		painter.setFont(QFont("Arial", 12, -1, true));
		posX = size.width()/3;
		posY = 300;		
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

	VirtualTouchScreen w;
	w.show();

	splashScr.finish(&w);

	return a.exec();
}
