#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QDebug>
#include <QPainter>
#include "PresenterHelper.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	QPixmap splashImg(":/icons/splash.jpg");
	if (!splashImg.isNull())
	{
		QPainter painter(&splashImg);
		painter.setPen(Qt::blue);
		QFont font("Arial", 30);
		painter.setFont(font);
		int posX = 290;
		int posY = 150;
		painter.drawText(posX, posY, "Presenter Helper");
		painter.drawText(posX-170, posY+60, "Based on Perceptual Computing");
		posX = 140;
		posY += 10;
		painter.setFont(QFont("Arial", 15));
		painter.drawText(posX+200, posY+100, "Available commands:");
		painter.drawText(posX+200, posY+130, " - F1: help");
		painter.drawText(posX+200, posY+160, " - F2: shows the configuration dialog");
		painter.drawText(posX+200, posY+190, " - F3: minimizes the cursor");
		painter.drawText(posX+200, posY+220, " - ctrl+q: quits the application");
		painter.setFont(QFont("Arial", 12, -1, true));
		painter.drawText(posX+200, posY+350, "Copyright Bogdan Cristea");
		painter.drawText(posX+190, posY+370, "e-mail: cristeab@gmail.com");
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
